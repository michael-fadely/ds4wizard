#include "pch.h"
#include <iomanip>
#include <sstream>

#include <shellapi.h>

#include <fmt/format.h>

#include <hid_util.h>
#include <devicetoggle.h>

#include "Ds4Device.h"
#include "lock.h"
#include "Logger.h"
#include "program.h"
#include "stringutil.h"

#include "Ds4DeviceManager.h"

Ds4DeviceManager::~Ds4DeviceManager()
{
	close();
}

bool Ds4DeviceManager::isDs4(const hid::HidInstance& hid)
{
	return hid.attributes().vendorId == vendorId &&
	       std::find(productIds.begin(), productIds.end(), hid.attributes().productId) != productIds.end();
}

bool Ds4DeviceManager::isDs4(const std::wstring& devicePath)
{
	hid::HidInstance hid(devicePath);
	return hid.readMetadata() && isDs4(hid);
}

void Ds4DeviceManager::findControllers()
{
	LOCK(sync);

	hid::enumerateHid([&](hid::HidInstance& hid) -> bool
	{
		return handleDevice(hid);
	});
}

void Ds4DeviceManager::findController(const std::wstring& devicePath)
{
	if (!isDs4(devicePath))
	{
		return;
	}

	GUID guid = {};
	HidD_GetHidGuid(&guid);

	LOCK(sync);

	hid::enumerateGuid([&](const std::wstring& path, const std::wstring& instanceId) -> bool
	{
		if (!iequals(path, devicePath))
		{
			return false;
		}

		hid::HidInstance hid(path, instanceId);

		if (hid.readMetadata())
		{
			return handleDevice(hid);
		}

		return false;
	}, guid);
}

size_t Ds4DeviceManager::deviceCount()
{
	LOCK(devices);
	return devices.size();
}

std::unique_lock<std::recursive_mutex> Ds4DeviceManager::lockDevices()
{
	std::unique_lock<std::recursive_mutex> devices_guard(devices_lock);
	return devices_guard;
}

bool Ds4DeviceManager::handleDevice(hid::HidInstance& hid)
{
	if (!isDs4(hid))
	{
		return false;
	}

	LOCK(sync);

	bool isBluetooth;

	try
	{
		isBluetooth = hid.caps().inputReportSize != 64;

		// USB connection type
		if (!isBluetooth)
		{
			// From DS4Windows
			std::array<uint8_t, 16> buffer {};
			buffer[0] = 18;

			if (!hid.getFeature(buffer))
			{
				throw /* TODO std::runtime_error(std::wstring.Format(Resources.DeviceReadMACFailed, hid.path)) */;
			}

			hid.serial =
			{
				buffer[6], buffer[5], buffer[4],
				buffer[3], buffer[2], buffer[1]
			};

			std::wstringstream serialString;

			serialString << std::setw(2) << std::setfill(L'0') << std::hex
				<< buffer[6] << buffer[5] << buffer[4] << buffer[3] << buffer[2] << buffer[1];

			hid.serialString = serialString.str();
		}

		if (hid.serialString.empty())
		{
			throw /* TODO std::runtime_error(std::wstring.Format(Resources.DeviceReturnedEmptyMAC, hid.Path)) */;
		}
	}
	catch (const std::exception& ex)
	{
		// TODO: proper HID exceptions
		Logger::writeLine(LogLevel::warning, "Failed to read device metadata: " + std::string(ex.what()));
		hid.close();
		return false;
	}

	try
	{
		LOCK(devices);

		const auto it = devices.find(hid.serialString);
		std::shared_ptr<Ds4Device> device;

		// device isn't already being managed, so set up all the event handling/etc
		if (it == devices.end())
		{
			device = std::make_shared<Ds4Device>();

			device->onDeviceClosed += [this](auto sender) { onDs4DeviceClosed(sender); };

			// TODO: translatable

			//device->onScpXInputHandleFailure    += [](auto sender) { Logger::writeLine(LogLevel::warning, "Failed to obtain ScpVBus XInput handle. XInput emulation will not be available."); };
			device->onBluetoothExclusiveFailure += [](auto sender) { Logger::writeLine(LogLevel::warning, sender->name(), "Failed to open Bluetooth device exclusively."); };
			device->onBluetoothConnected        += [](auto sender) { Logger::writeLine(LogLevel::info,    sender->name(), "Bluetooth connected."); };
			device->onBluetoothIdleDisconnect   += [](auto sender) { Logger::writeLine(LogLevel::info,    sender->name(), "Bluetooth idle disconnect." /* TODO: std::string.Format(Resources.IdleDisconnect, idleTimeout)*/); };
			device->onBluetoothDisconnected     += [](auto sender) { Logger::writeLine(LogLevel::info,    sender->name(), "Bluetooth disconnected."); };
			device->onUsbExclusiveFailure       += [](auto sender) { Logger::writeLine(LogLevel::warning, sender->name(), "Failed to open USB device exclusively."); };
			device->onUsbConnected              += [](auto sender) { Logger::writeLine(LogLevel::info,    sender->name(), "USB connected."); };

			device->onLatencyThresholdExceeded += [](auto sender, std::chrono::milliseconds value, std::chrono::milliseconds threshold)
			{
				const std::string str = fmt::format("Input latency has exceeded the threshold. ({0} ms > {1} ms)", value.count(), threshold.count());
				Logger::writeLine(LogLevel::warning, sender->name(), str);
			};

			// TODO: don't toggle the device unless opening exclusively fails!
			toggleDevice(hid.instanceId);
			device->open(hid);

			const auto& safe = device->safeMacAddress();

			const std::wstring wstr(safe.cbegin(), safe.cend());
			devices[wstr] = device;

			auto args = std::make_shared<DeviceOpenedEventArgs>(device, true);
			deviceOpened.invoke(this, args);
			device->start();
		}
		else
		{
			device = it->second;

			if (isBluetooth)
			{
				if (!device->bluetoothConnected())
				{
					// TODO: don't toggle the device unless opening exclusively fails!
					toggleDevice(hid.instanceId);
					device->openBluetoothDevice(hid);
				}
			}
			else
			{
				if (!device->usbConnected())
				{
					// TODO: don't toggle the device unless opening exclusively fails!
					toggleDevice(hid.instanceId);
					device->openUsbDevice(hid);
				}
			}

			auto args = std::make_shared<DeviceOpenedEventArgs>(device, false);
			deviceOpened.invoke(this, args);
		}
	}
	catch (const std::exception& ex)
	{
		// TODO: proper HID exceptions
		Logger::writeLine(LogLevel::error, "Error while opening device: " + std::string(ex.what()));
	}

	return false;
}

void Ds4DeviceManager::onDs4DeviceClosed(Ds4Device* sender)
{
	LOCK(devices);

	const auto mac = std::wstring(sender->safeMacAddress().cbegin(), sender->safeMacAddress().cend());
	const auto it = devices.find(mac);

	if (it == devices.end())
	{
		return;
	}

	std::shared_ptr<Ds4Device> ptr = it->second;

	auto args = std::make_shared<DeviceClosedEventArgs>(ptr);
	deviceClosed.invoke(this, args);
	devices.erase(it);
}

void Ds4DeviceManager::close()
{
	decltype(devices) devices_;

	{
		LOCK(devices);
		devices_ = std::move(devices);
	}

	for (auto& it : devices_)
	{
		const std::shared_ptr<Ds4Device> ptr = std::move(it.second);
		ptr->close();

		auto args = std::make_shared<DeviceClosedEventArgs>(ptr);
		deviceClosed.invoke(this, args);
	}
}

void Ds4DeviceManager::toggleDevice(const std::wstring& instanceId)
{
	// TODO: check if windows 8 or newer, otherwise don't bother

	if (Program::isElevated())
	{
		::toggleDevice(instanceId);
		return;
	}

	std::wstring params = L"--toggle-device \"";
	params.append(instanceId);
	params.append(L"\"");

	SHELLEXECUTEINFO info {};
	info.cbSize       = sizeof(info);
	info.fMask        = SEE_MASK_NOCLOSEPROCESS;
	info.lpFile       = L"ds4wizard-device-toggle.exe";
	info.lpParameters = params.c_str();
	info.lpVerb       = L"runas";
	//info.nShow        = SW_SHOW;

	if (!ShellExecuteExW(&info))
	{
		throw;
	}

	const Handle handle(info.hProcess, true);

	WaitForSingleObject(handle.nativeHandle, INFINITE);
}
