#include "stdafx.h"
#include <iomanip>
#include <sstream>

#include <shellapi.h>
#include <hid_util.h>
#include <devicetoggle.h>

#include "Ds4Device.h"
#include "lock.h"
#include "Logger.h"
#include "program.h"

#include "Ds4DeviceManager.h"

Ds4DeviceManager::~Ds4DeviceManager()
{
	close();
}

bool Ds4DeviceManager::isDs4(const hid::HidInstance& hid)
{
	if (hid.attributes().vendorId == vendorId)
	{
		for (auto id : productIds)
		{
			if (id == hid.attributes().productId)
			{
				return true;
			}
		}
	}

	return false;
}

bool Ds4DeviceManager::isDs4(const std::wstring& devicePath)
{
	const hid::HidInstance hid(devicePath, true);
	return isDs4(hid);
}

void Ds4DeviceManager::findControllers()
{
	lock(sync);

	hid::enumerateHid([&](hid::HidInstance& hid) -> bool
	{
		if (hid.attributes().vendorId != vendorId)
		{
			return false;
		}

		if (std::find(productIds.begin(), productIds.end(), hid.attributes().productId) == productIds.end())
		{
			return false;
		}

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

		hid.close(); // HACK: this should not be required; why is it open???

		try
		{
			lock(devices);

			const auto it = devices.find(hid.serialString);
			std::shared_ptr<Ds4Device> device;

			if (it == devices.end())
			{
				// TODO: don't toggle the device unless opening exclusively fails!
				queueDeviceToggle(hid.instanceId);

				device = std::make_shared<Ds4Device>(hid);
				device->deviceClosed += std::bind(&Ds4DeviceManager::onDs4DeviceClosed, this,
				                                  std::placeholders::_1, std::placeholders::_2);

				DeviceOpenedEventArgs args(device, true);
				onDeviceOpened(args);
				device->start();

				auto& safe = device->safeMacAddress;
				const std::wstring wstr(safe.begin(), safe.end());
				devices[wstr] = device;
			}
			else
			{
				device = it->second;

				if (isBluetooth)
				{
					if (!device->bluetoothConnected())
					{
						// TODO: don't toggle the device unless opening exclusively fails!
						queueDeviceToggle(hid.instanceId);
						device->openBluetoothDevice(hid);
					}
				}
				else
				{
					if (!device->usbConnected())
					{
						// TODO: don't toggle the device unless opening exclusively fails!
						queueDeviceToggle(hid.instanceId);
						device->openUsbDevice(hid);
					}
				}

				DeviceOpenedEventArgs args(device, false);
				onDeviceOpened(args);
			}
		}
		catch (const std::exception& ex)
		{
			// TODO: proper HID exceptions
			Logger::writeLine(LogLevel::error, "Error while opening device: " + std::string(ex.what()));
		}

		return false;
	});
}

void Ds4DeviceManager::onDs4DeviceClosed(void* sender, EventArgs*)
{
	lock(devices);

	auto raw_ptr = reinterpret_cast<Ds4Device*>(sender);

	const auto macaddr = std::wstring(raw_ptr->safeMacAddress.begin(), raw_ptr->safeMacAddress.end());
	auto it = devices.find(macaddr);

	if (it == devices.end())
	{
		return;
	}

	std::shared_ptr<Ds4Device> ptr = it->second;

	DeviceClosedEventArgs args(ptr);
	onDeviceClosed(args);
	devices.erase(it);
}

void Ds4DeviceManager::close()
{
	decltype(devices) devices_;
	{
		lock(devices);
		devices_ = std::move(devices);
	}

	for (auto& it : devices_)
	{
		const std::shared_ptr<Ds4Device> ptr = std::move(it.second);
		ptr->close();

		DeviceClosedEventArgs args(ptr);
		onDeviceClosed(args);
	}
}

void Ds4DeviceManager::queueDeviceToggle(const std::wstring& instanceId)
{
	toggleDeviceElevated(instanceId);
}

void Ds4DeviceManager::toggleDeviceElevated(const std::wstring& instanceId)
{
	if (Program::isElevated())
	{
		toggleDevice(instanceId);
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
	info.nShow        = SW_SHOW;

	if (!ShellExecuteExW(&info))
	{
		throw;
	}

	Handle handle(info.hProcess, true);

	WaitForSingleObject(handle.nativeHandle, INFINITE);
}

void Ds4DeviceManager::toggleDevice(const std::wstring& instanceId)
{
	::toggleDevice(instanceId);
}

void Ds4DeviceManager::onDeviceOpened(DeviceOpenedEventArgs& e)
{
	deviceOpened.invoke(this, &e);
}

void Ds4DeviceManager::onDeviceClosed(DeviceClosedEventArgs& e)
{
	deviceClosed.invoke(this, &e);
}
