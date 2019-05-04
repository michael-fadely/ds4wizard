#include <iomanip>
#include <sstream>

#include <Windows.h>
#include <SetupAPI.h>
#include <hidsdi.h>

#include "devicetoggle.h"

void toggleDevice_doWork(const std::wstring& instanceId, const HDEVINFO devInfoSet)
{
	SP_DEVINFO_DATA devInfoData;
	devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

	if (!SetupDiEnumDeviceInfo(devInfoSet, 0, &devInfoData))
	{
		const DWORD error = GetLastError();

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
		const DWORD error = GetLastError();

		std::stringstream message;
		message << "Failed to set class install parameters for device "
			<< std::string(instanceId.begin(), instanceId.end()) << " with error code " << std::hex << std::setw(8) << std::setfill('0') << error;

		throw std::runtime_error(message.str());
	}

	// Apply the changes, disabling the device.
	success = SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, devInfoSet, &devInfoData);

	if (!success)
	{
		const DWORD error = GetLastError();

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
		const DWORD error = GetLastError();

		std::stringstream message;
		message << "Failed to set class install parameters for device "
			<< std::string(instanceId.begin(), instanceId.end()) << " with error code " << std::hex << std::setw(8) << std::setfill('0') << error;

		throw std::runtime_error(message.str());
	}

	// Apply the changes, enabling the device.
	success = SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, devInfoSet, &devInfoData);

	if (!success)
	{
		const DWORD error = GetLastError();

		std::stringstream message;
		message << "Failed to enable device "
			<< std::string(instanceId.begin(), instanceId.end()) << " with error code " << std::hex << std::setw(8) << std::setfill('0') << error;

		throw std::runtime_error(message.str());
	}
}

void toggleDevice(const std::wstring& instanceId)
{
	GUID guid;
	HidD_GetHidGuid(&guid);
	const HDEVINFO devInfoSet = SetupDiGetClassDevs(&guid, instanceId.c_str(), nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	if (devInfoSet == nullptr || devInfoSet == INVALID_HANDLE_VALUE)
	{
		const DWORD error = GetLastError();

		std::stringstream message;
		message << "SetupDiGetClassDevs failed for device "
			<< std::string(instanceId.begin(), instanceId.end()) << " with error code " << std::hex << std::setw(8) << std::setfill('0') << error;

		throw std::runtime_error(message.str());
	}

	const auto release = [&]()
	{
		SetupDiDestroyDeviceInfoList(devInfoSet);
	};

	try
	{
		toggleDevice_doWork(instanceId, devInfoSet);
	}
	catch (const std::exception&)
	{
		release();
		throw;
	}

	release();
}
