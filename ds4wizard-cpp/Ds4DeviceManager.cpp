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

/*
 * TODO: this needs to be re-done; see reasons below
 * - when a device fails to open and is toggled, it closes and re-opens the xinput handle;
 *   ideally it should not open the xinput handle until the device is opened, exclusive or not.
 * - a cooldown time should be specified on toggles for a device.
 * - a "device open failed" notification can be displayed immediately followed by successful device open.
 */

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

	hid::enumerateHid([&](std::shared_ptr<hid::HidInstance> hid) -> bool
	{
		return handleDevice(std::move(hid));
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

		auto hid = std::make_shared<hid::HidInstance>(path, instanceId);

		if (hid->readMetadata())
		{
			return handleDevice(std::move(hid));
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
	return std::unique_lock<std::recursive_mutex>(devices_lock);
}

bool Ds4DeviceManager::handleDevice(std::shared_ptr<hid::HidInstance> hid)
{
	if (!isDs4(*hid))
	{
		return false;
	}

	LOCK(sync);

	bool isBluetooth;

	try
	{
		isBluetooth = hid->caps().inputReportSize != 64;

		// USB connection type
		if (!isBluetooth)
		{
			// From DS4Windows
			std::array<uint8_t, 16> buffer {};
			buffer[0] = 18;

			if (!hid->getFeature(buffer))
			{
				const std::string hidPath(hid->path.begin(), hid->path.end());
				throw std::runtime_error(fmt::format("Failed to read MAC address from USB device {0}", hidPath));
			}

			hid->serial =
			{
				buffer[6], buffer[5], buffer[4],
				buffer[3], buffer[2], buffer[1]
			};

			std::wstringstream serialString;

			serialString << std::setw(2) << std::setfill(L'0') << std::hex
				<< buffer[6] << buffer[5] << buffer[4] << buffer[3] << buffer[2] << buffer[1];

			hid->serialString = serialString.str();
		}

		if (hid->serialString.empty())
		{
			const std::string hidPath(hid->path.begin(), hid->path.end());
			throw std::runtime_error(fmt::format("Device {0} returned empty MAC address.", hidPath));
		}
	}
	catch (const std::exception& ex)
	{
		// TODO: proper HID exceptions
		Logger::writeLine(LogLevel::warning, "Failed to read device metadata: " + std::string(ex.what()));
		hid->close();
		return false;
	}

	try
	{
		LOCK(devices);

		const auto it = devices.find(hid->serialString);

		// device isn't already being managed, so set up all the event handling/etc
		if (it == devices.end())
		{
			std::shared_ptr<Ds4Device> device = std::make_shared<Ds4Device>();
			auto& token_store = tokens.insert({ hid->serialString, {} }).first->second;

			token_store.push_back(device->onDeviceClosed.add([this](auto sender) { onDs4DeviceClosed(sender); }));

			bool toggledUsb = false;
			bool toggledBluetooth = false;

			auto onUsbExclusiveFailure = [&](Ds4Device* sender)
			{
				if (toggledUsb)
				{
					// TODO: translatable
					Logger::writeLine(LogLevel::warning, sender->name(), "Failed to open USB device exclusively.");
					return;
				}

				toggledUsb = true;
				toggleDevice(hid->instanceId);
				device->openUsbDevice(hid);
			};

			auto onBluetoothExclusiveFailure = [&](Ds4Device* sender)
			{
				if (toggledBluetooth)
				{
					// TODO: translatable
					Logger::writeLine(LogLevel::warning, sender->name(), "Failed to open Bluetooth device exclusively.");
					return;
				}

				toggledBluetooth = true;
				toggleDevice(hid->instanceId);
			};

			token_store.push_back(device->onBluetoothExclusiveFailure.add(onBluetoothExclusiveFailure));
			token_store.push_back(device->onUsbExclusiveFailure.add(onUsbExclusiveFailure));

			// TODO: translatable
			token_store.push_back(device->onBluetoothConnected       .add([](auto sender) { Logger::writeLine(LogLevel::info,    sender->name(), "Bluetooth connected."); }));
			token_store.push_back(device->onBluetoothIdleDisconnect  .add([](auto sender) { Logger::writeLine(LogLevel::info,    sender->name(), "Bluetooth idle disconnect."); }));
			token_store.push_back(device->onBluetoothDisconnected    .add([](auto sender) { Logger::writeLine(LogLevel::info,    sender->name(), "Bluetooth disconnected."); }));
			token_store.push_back(device->onUsbExclusiveFailure      .add([](auto sender) { Logger::writeLine(LogLevel::warning, sender->name(), "Failed to open USB device exclusively."); }));
			token_store.push_back(device->onUsbConnected             .add([](auto sender) { Logger::writeLine(LogLevel::info,    sender->name(), "USB connected."); }));

			token_store.push_back(device->onLatencyThresholdExceeded.add(
			[](auto sender, std::chrono::milliseconds value, std::chrono::milliseconds threshold)
			{
				const std::string str = fmt::format("Input latency has exceeded the threshold. ({0} ms > {1} ms)", value.count(), threshold.count());
				Logger::writeLine(LogLevel::warning, sender->name(), str);
			}));

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
			std::shared_ptr<Ds4Device> device = it->second;

			if (isBluetooth)
			{
				if (!device->bluetoothConnected())
				{
					device->openBluetoothDevice(hid);
				}
			}
			else
			{
				if (!device->usbConnected())
				{
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

		LOCK(devices);
		devices.erase(hid->serialString);
		tokens.erase(hid->serialString);
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
	tokens.erase(mac);
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

		const auto mac = std::wstring(ptr->safeMacAddress().cbegin(), ptr->safeMacAddress().cend());
		tokens.erase(mac);
	}
}

void Ds4DeviceManager::toggleDevice(const std::wstring& instanceId)
{
	// TODO: check if windows 8 or newer, otherwise don't bother
	// TODO: use CreateProcess and get console output

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

	if (!ShellExecuteExW(&info))
	{
		throw std::runtime_error("ShellExecuteExW failed");
	}

	Handle handle(info.hProcess, true);

	WaitForSingleObject(handle.nativeHandle, INFINITE);

	DWORD exitCode = 0;
	if (!GetExitCodeProcess(handle.nativeHandle, &exitCode) || exitCode != 0)
	{
		handle.close();
		throw std::runtime_error("Device toggle failed! Please report.");
	}
}
