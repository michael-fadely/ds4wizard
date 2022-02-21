#include <iomanip>
#include <sstream>
#include <codecvt>

#include <Windows.h>
#include <SetupAPI.h>
#include <hidsdi.h>

#include "devicetoggle.h"

std::string getLastErrorString(const DWORD error)
{
	if (error == ERROR_SUCCESS)
	{
		return {};
	}

	LPSTR messageBuffer = nullptr;

	const DWORD size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
	                                  NULL,
	                                  error,
	                                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
	                                  reinterpret_cast<LPSTR>(&messageBuffer),
	                                  0,
	                                  NULL);

	if (size < 1 && GetLastError() != ERROR_SUCCESS)
	{
		return {};
	}

	std::string message(messageBuffer, size);

	LocalFree(messageBuffer);

	return message;
}

std::string getLastErrorString()
{
	return getLastErrorString(GetLastError());
}

inline void wideToStringError(int length)
{
	if (length < 1)
	{
		switch (GetLastError())
		{
			case ERROR_SUCCESS:
				break;

			default:
				throw std::runtime_error(getLastErrorString());
		}
	}
}

std::string wideToString(const std::wstring& wideString)
{
	if (!wideString.length())
	{
		return {};
	}

	constexpr UINT codePage = CP_UTF8;
	constexpr DWORD flags = WC_ERR_INVALID_CHARS;

	const int requiredBufferSize = WideCharToMultiByte(codePage,
	                                                   flags,
	                                                   wideString.data(),
	                                                   static_cast<int>(wideString.length()),
	                                                   nullptr,
	                                                   0,
	                                                   NULL,
	                                                   NULL);

	wideToStringError(requiredBufferSize);

	std::string result(requiredBufferSize, '\0');

	const int converted = WideCharToMultiByte(codePage,
	                                          flags,
	                                          wideString.data(),
	                                          static_cast<int>(wideString.length()),
	                                          result.data(),
	                                          requiredBufferSize,
	                                          NULL,
	                                          NULL);

	wideToStringError(converted);

	return result;
}

void toggleDevice(const std::wstring& instanceId)
{
	GUID guid;
	HidD_GetHidGuid(&guid);
	const HDEVINFO devInfoSet = SetupDiGetClassDevs(&guid, instanceId.c_str(), nullptr,
	                                                DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	if (devInfoSet == nullptr || devInfoSet == INVALID_HANDLE_VALUE)
	{
		const DWORD error = GetLastError();
		std::stringstream message;

		message << "SetupDiGetClassDevs failed for device "
			<< wideToString(instanceId) << " with error code "
			<< std::hex << std::setw(8) << std::setfill('0') << error << ": " << getLastErrorString(error);

		throw std::runtime_error(message.str());
	}

	const auto release = [&]()
	{
		SetupDiDestroyDeviceInfoList(devInfoSet);
	};

	try
	{
		SP_DEVINFO_DATA devInfoData;
		devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

		if (!SetupDiEnumDeviceInfo(devInfoSet, 0, &devInfoData))
		{
			const DWORD error = GetLastError();
			std::stringstream message;

			message << "Retrieving device info for instance ID "
				<< wideToString(instanceId) << " failed while attempting to toggle device with error code "
				<< std::hex << std::setw(8) << std::setfill('0') << error << ": " << getLastErrorString(error);

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
		                                            reinterpret_cast<PSP_CLASSINSTALL_HEADER>(&propChangeParams),
		                                            sizeof(propChangeParams));

		if (!success)
		{
			const DWORD error = GetLastError();
			std::stringstream message;

			message << "Failed to set class install parameters for device "
				<< wideToString(instanceId) << " with error code "
				<< std::hex << std::setw(8) << std::setfill('0') << error << ": " << getLastErrorString(error);

			throw std::runtime_error(message.str());
		}

		// Apply the changes, disabling the device.
		success = SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, devInfoSet, &devInfoData);

		if (!success)
		{
			const DWORD error = GetLastError();
			std::stringstream message;

			message << "Failed to disable device "
				<< wideToString(instanceId) << " with error code "
				<< std::hex << std::setw(8) << std::setfill('0') << error << ": " << getLastErrorString(error);

			throw std::runtime_error(message.str());
		}

		// Change state back to enabled.
		propChangeParams.StateChange = DICS_ENABLE;

		// Prepare the class install parameters again.
		success = SetupDiSetClassInstallParams(devInfoSet, &devInfoData,
		                                       reinterpret_cast<PSP_CLASSINSTALL_HEADER>(&propChangeParams),
		                                       sizeof(propChangeParams));

		if (!success)
		{
			const DWORD error = GetLastError();
			std::stringstream message;

			message << "Failed to set class install parameters for device "
				<< wideToString(instanceId) << " with error code "
				<< std::hex << std::setw(8) << std::setfill('0') << error << ": " << getLastErrorString(error);

			throw std::runtime_error(message.str());
		}

		// Apply the changes, enabling the device.
		success = SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, devInfoSet, &devInfoData);

		if (!success)
		{
			const DWORD error = GetLastError();
			std::stringstream message;

			message << "Failed to enable device "
				<< wideToString(instanceId) << " with error code "
				<< std::hex << std::setw(8) << std::setfill('0') << error << ": " << getLastErrorString(error);

			throw std::runtime_error(message.str());
		}
	}
	catch (const std::exception&)
	{
		release();
		throw;
	}

	release();
}
