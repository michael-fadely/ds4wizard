#include "stdafx.h"
#include "Ds4DeviceManager.h"
#include "lock.h"
#include <hid_util.h>
#include <sstream>
#include "program.h"
#include <shellapi.h>
#include "Ds4Device.h"
#include <iomanip>
#include "Logger.h"

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

	hid::enumerateHID([&](hid::HidInstance& hid) -> bool
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

		try
		{
			lock(devices);

			const auto it = devices.find(hid.serialString);
			std::shared_ptr<Ds4Device> device = nullptr;

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
				auto wstr = std::wstring(safe.begin(), safe.end());
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

	it = devices.erase(it);
}

void Ds4DeviceManager::close()
{
	lock(devices);

	// shared_ptr will run the destructors and cause all the devices to close
	devices.clear();
}

void Ds4DeviceManager::queueDeviceToggle(const std::wstring& instanceId)
{
	return; // HACK !!!!!!!!
	toggleDeviceElevated(instanceId);
}

void Ds4DeviceManager::toggleDeviceElevated(const std::wstring& instanceId)
{
	if (Program::isElevated())
	{
		toggleDevice(instanceId);
		return;
	}

	std::wstring currentProcess = QCoreApplication::applicationFilePath().toStdWString();

	std::wstring params = L"--toggle-device \"";
	params.append(instanceId);
	params.append(L"\"");

	HINSTANCE hinst = ShellExecuteW(nullptr, L"runas",
	                                currentProcess.c_str(), params.c_str(), nullptr, SW_SHOWDEFAULT);

	WaitForSingleObject(hinst, INFINITE);
}

void Ds4DeviceManager::toggleDevice(const std::wstring& instanceId)
{
	GUID guid;
	HidD_GetHidGuid(&guid);
	HDEVINFO devInfoSet = SetupDiGetClassDevs(&guid, instanceId.c_str(), nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	SP_DEVINFO_DATA devInfoData;
	devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

	if (!SetupDiEnumDeviceInfo(devInfoSet, 0, &devInfoData))
	{
		DWORD error = GetLastError();

		std::stringstream message;
		message << "Retrieving device info for instance ID "
			<< std::string(instanceId.begin(), instanceId.end()) << " failed while attempting to toggle device with error code " << std::hex << std::setw(8) << std::setfill('0') << error;

		throw std::runtime_error(message.str());
	}

	SP_PROPCHANGE_PARAMS propChangeParams;

	propChangeParams.ClassInstallHeader.cbSize          = sizeof(SP_CLASSINSTALL_HEADER);
	propChangeParams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;

	propChangeParams.StateChange = DICS_DISABLE;
	propChangeParams.HwProfile   = 0;
	propChangeParams.Scope       = DICS_FLAG_GLOBAL;

	// Prepare the device by setting the class install parameters.
	bool success = SetupDiSetClassInstallParams(devInfoSet, &devInfoData,
	                                            reinterpret_cast<PSP_CLASSINSTALL_HEADER>(&propChangeParams), sizeof(propChangeParams));

	if (!success)
	{
		DWORD error = GetLastError();

		std::stringstream message;
		message << "Failed to set class install parameters for device "
			<< std::string(instanceId.begin(), instanceId.end()) << " with error code " << std::hex << std::setw(8) << std::setfill('0') << error;

		throw std::runtime_error(message.str());
	}

	// Apply the changes, disabling the device.
	success = SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, devInfoSet, &devInfoData);

	if (!success)
	{
		DWORD error = GetLastError();

		std::stringstream message;
		message << "Failed to disable device "
			<< std::string(instanceId.begin(), instanceId.end()) << " with error code " << std::hex << std::setw(8) << std::setfill('0') << error;

		throw std::runtime_error(message.str());
	}

	// Change state back to enabled.
	propChangeParams.StateChange = DICS_ENABLE;

	// Prepare the class install parameters again.
	success = SetupDiSetClassInstallParams(devInfoSet, &devInfoData,
	                                       reinterpret_cast<PSP_CLASSINSTALL_HEADER>(&propChangeParams), sizeof(propChangeParams));

	if (!success)
	{
		DWORD error = GetLastError();

		std::stringstream message;
		message << "Failed to set class install parameters for device "
			<< std::string(instanceId.begin(), instanceId.end()) << " with error code " << std::hex << std::setw(8) << std::setfill('0') << error;

		throw std::runtime_error(message.str());
	}

	// Apply the changes, enabling the device.
	success = SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, devInfoSet, &devInfoData);

	if (!success)
	{
		DWORD error = GetLastError();

		std::stringstream message;
		message << "Failed to enable device "
			<< std::string(instanceId.begin(), instanceId.end()) << " with error code " << std::hex << std::setw(8) << std::setfill('0') << error;

		throw std::runtime_error(message.str());
	}

	SetupDiDestroyDeviceInfoList(devInfoSet);
}

void Ds4DeviceManager::onDeviceOpened(DeviceOpenedEventArgs& e)
{
	DeviceOpened.invoke(this, &e);
}

void Ds4DeviceManager::onDeviceClosed(DeviceClosedEventArgs& e)
{
	DeviceClosed.invoke(this, &e);
}
