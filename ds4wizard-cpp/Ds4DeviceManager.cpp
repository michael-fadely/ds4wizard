#include "stdafx.h"
#include "Ds4DeviceManager.h"
#include "lock.h"
#include "util.h"
#include <sstream>
#include "program.h"
#include <shellapi.h>
#include "Ds4Device.h"

bool Ds4DeviceManager::IsDs4(const hid::HidInstance& hid)
{
	if (hid.attributes().vendor_id == Ds4VendorID)
	{
		for (auto id : Ds4ProductIDs)
		{
			if (id == hid.attributes().product_id)
			{
				return true;
			}
		}
	}

	return false;
}

bool Ds4DeviceManager::IsDs4(const std::wstring& devicePath)
{
	const hid::HidInstance hid(devicePath, true);
	return IsDs4(hid);
}

void Ds4DeviceManager::FindControllers()
{
	lock(sync);

	hid::enum_hid([&](hid::HidInstance& hid) -> bool
	{
		if (hid.attributes().vendor_id != Ds4VendorID)
		{
			return false;
		}

		if (std::find(Ds4ProductIDs.begin(), Ds4ProductIDs.end(), hid.attributes().product_id) == Ds4ProductIDs.end())
		{
			return false;
		}

		bool isBluetooth;

		try
		{
			isBluetooth = hid.caps().input_report_size != 64;

			// USB connection type
			if (!isBluetooth)
			{
				// From DS4Windows
				std::array<uint8_t, 16> buffer {};
				buffer[0] = 18;

				if (!hid.get_feature(buffer))
				{
					throw /*std::runtime_error(std::wstring.Format(Resources.DeviceReadMACFailed, hid.path)) // TODO */;
				}

				hid.serial =
				{
					buffer[6], buffer[5], buffer[4],
					buffer[3], buffer[2], buffer[1]
				};

				std::wstringstream serialString;

				serialString << std::setw(2) << std::setfill(L'0') << std::hex
					<< buffer[6] << buffer[5] << buffer[4] << buffer[3] << buffer[2] << buffer[1];

				hid.serial_string = serialString.str();
			}

			if (hid.serial_string.empty())
			{
				throw /*std::runtime_error(std::wstring.Format(Resources.DeviceReturnedEmptyMAC, hid.Path)) // TODO */;
			}
		}
		catch (const std::exception&)
		{
			// TODO: proper HID exceptions
			//Logger.WriteLine(LogLevel.Warning, $"Failed to read device metadata: {ex.Message}"); // TODO
			hid.close();
			return false;
		}

		try
		{
			lock(devices);

			const auto it = devices.find(hid.serial_string);
			std::shared_ptr<Ds4Device> device = nullptr;

			if (it == devices.end())
			{
				QueueDeviceToggle(hid.instance_id);
				device = std::make_shared<Ds4Device>(hid);
				//device->DeviceClosed += OnDs4DeviceClosed; // TODO

				//OnDeviceOpened(new DeviceOpenedEventArgs(device, true)); // TODO
				device->Start();

				auto& safe = device->SafeMacAddress;
				devices[std::wstring(safe.begin(), safe.end())] = device;
			}
			else
			{
				if (isBluetooth)
				{
					if (!device->BluetoothConnected())
					{
						QueueDeviceToggle(hid.instance_id);
						device->OpenBluetoothDevice(hid);
					}
				}
				else
				{
					if (!device->UsbConnected())
					{
						QueueDeviceToggle(hid.instance_id);
						device->OpenUsbDevice(hid);
					}
				}

				//OnDeviceOpened(new DeviceOpenedEventArgs(device, false)); // TODO
			}
		}
		catch (const std::exception& ex)
		{
			qDebug() << ex.what();
			// TODO: proper HID exceptions
			//Logger.WriteLine(LogLevel.Error, $"Error while opening device: {ex.Message}"); // TODO
		}

		return false;
	});
}

void Ds4DeviceManager::Close()
{
	lock(devices);

	for (auto& pair : devices)
	{
		pair.second->Close();
	}

	devices.clear();
}

void Ds4DeviceManager::QueueDeviceToggle(const std::wstring& instanceId)
{
	ToggleDeviceElevated(instanceId);
}

void Ds4DeviceManager::ToggleDeviceElevated(const std::wstring& instanceId)
{
	if (Program::isElevated())
	{
		ToggleDevice(instanceId);
		return;
	}

	/*string currentProcess = Assembly.GetEntryAssembly().Location;

	var info = new ProcessStartInfo(currentProcess)
	{
		UseShellExecute = true,
		Verb = "runas",
		Arguments = $"--toggle-device \"{instanceId}\""
	};

	using (Process p = Process.Start(info))
	{
		p?.WaitForExit();
	}*/

	std::wstring currentProcess = QCoreApplication::applicationFilePath().toStdWString();

	std::wstring params = L"--toggle-device \"";
	params.append(instanceId);
	params.append(L"\"");

	HINSTANCE hinst = ShellExecuteW(nullptr, L"runas",
	                                currentProcess.c_str(), params.c_str(), nullptr, SW_SHOWDEFAULT);

	WaitForSingleObject(hinst, INFINITE);
}

void Ds4DeviceManager::ToggleDevice(const std::wstring& instanceId)
{
	GUID guid;
	HidD_GetHidGuid(&guid);
	auto devInfoSet = SetupDiGetClassDevsW(&guid, instanceId.c_str(), nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

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

	SP_PROPCHANGE_PARAMS propChangeParams {};

	SP_CLASSINSTALL_HEADER& cih = propChangeParams.ClassInstallHeader;
	cih.cbSize                  = sizeof(SP_CLASSINSTALL_HEADER);
	cih.InstallFunction         = DIF_PROPERTYCHANGE;

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

	// Apply the changes, disabling the device->
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

	// Apply the changes, enabling the device->
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
