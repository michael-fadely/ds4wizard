#include "stdafx.h"
#include <windows.h>
#include "ScpDevice.h"
#include <SetupAPI.h>
#include "busenum.h"
#include "lock.h"

ptrdiff_t ScpDevice::getFreePort()
{
	lock(portLock);
	{
		for (size_t i = 0; i < portsUsed.size(); i++)
		{
			if (!portsUsed[i])
			{
				return i;
			}
		}
	}

	return -1;
}

ScpDevice::ScpDevice(Handle&& handle)
{
	this->handle = std::move(handle);

	uint64_t version;
	if (getDriverVersion(version) != 0)
	{
		return;
	}

	driverVersion[0] = static_cast<short>(version >> 48);
	driverVersion[1] = static_cast<short>((version >> 32) & 0xFFFF);
	driverVersion[2] = static_cast<short>((version >> 16) & 0xFFFF);
	driverVersion[3] = static_cast<short>(version & 0xFFFF);
}

ScpDevice::~ScpDevice()
{
	close();
}

int ScpDevice::getDriverVersion(uint64_t& version)
{
	version = 0;

	auto devInfoSet = SetupDiGetClassDevs(&GUID_DEVINTERFACE_SCPVBUS, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	if (devInfoSet == INVALID_HANDLE_VALUE)
	{
		return GetLastError();
	}

	SP_DEVINFO_DATA devInfo {};
	devInfo.cbSize = sizeof(SP_DEVINFO_DATA);

	if (!SetupDiEnumDeviceInfo(devInfoSet, 0, &devInfo))
	{
		return GetLastError();
	}

	SP_DEVINSTALL_PARAMS p {};
	p.cbSize = sizeof(SP_DEVINSTALL_PARAMS);

	if (!SetupDiGetDeviceInstallParams(devInfoSet, &devInfo, &p))
	{
		return GetLastError();
	}

	p.FlagsEx |= DI_FLAGSEX_INSTALLEDDRIVER;

	if (!SetupDiSetDeviceInstallParams(devInfoSet, &devInfo, &p))
	{
		return GetLastError();
	}

	// SPDIT_COMPATDRIVER
	if (!SetupDiBuildDriverInfoList(devInfoSet, &devInfo, 2))
	{
		return GetLastError();
	}

	SP_DRVINFO_DATA drvInfo {};
	drvInfo.cbSize = sizeof(SP_DRVINFO_DATA);

	if (!SetupDiEnumDriverInfo(devInfoSet, &devInfo, 2, 0, &drvInfo))
	{
		return GetLastError();
	}

	version = drvInfo.DriverVersion;
	return 0;
}

void ScpDevice::close()
{
	handle.close();
}

bool ScpDevice::connect(int userIndex)
{
	if (userIndex < 0 || userIndex > 3)
	{
		throw /*new IndexOutOfRangeException("User index must be in the range 0 to 3.") // TODO */;
	}

	auto buffer = BUSENUM_UNPLUG_HARDWARE::create(userIndex);

	const int flags = driverVersion[0] < 17 ? 0x2A4000 : 0x2AA004;

	bool result = DeviceIoControl(handle.nativeHandle, flags,
	                              &buffer, buffer.Size, nullptr, 0, nullptr, nullptr);

	if (result)
	{
		lock(portLock);
		{
			portsUsed[userIndex] = true;
		}
	}

	return result;
}

bool ScpDevice::disconnect(int userIndex, bool force)
{
	if (userIndex < -1 || userIndex > 3)
	{
		throw /*new IndexOutOfRangeException("User index must be in the range -1 to 3.") // TODO */;
	}

	if (userIndex >= 0)
	{
		lock(portLock);
		{
			portsUsed[userIndex] = false;
		}
	}
	else
	{
		lock(portLock);
		{
			for (bool& i : portsUsed)
			{
				i = false;
			}
		}
	}

	auto buffer = BUSENUM_UNPLUG_HARDWARE::create(userIndex);

	if (driverVersion[0] >= 17)
	{
		buffer.Flags = force ? 0x0001 : 0x0000;
	}

	const int flags = driverVersion[0] < 17 ? 0x2A4004 : 0x2AA008;

	return DeviceIoControl(handle.nativeHandle, flags,
	                       &buffer, buffer.Size, nullptr, 0, nullptr, nullptr);
}

VBusStatus ScpDevice::syncState(int userIndex, const XInputGamepad& gamepad)
{
	gamepads[userIndex] = gamepad;
	return syncState(userIndex);
}

VBusStatus ScpDevice::syncState(int userIndex)
{
	const int busIndex = userIndex + 1;

	// This would be BUSENUM_REPORT_HARDWARE but
	// I can't be bothered to deal with the embedded
	// buffer.

	writeBuffer[0] = 0x1C;

	writeBuffer[4] = static_cast<uint8_t>((busIndex >> 0) & 0xFF);
	writeBuffer[5] = static_cast<uint8_t>((busIndex >> 8) & 0xFF);
	writeBuffer[6] = static_cast<uint8_t>((busIndex >> 16) & 0xFF);
	writeBuffer[7] = static_cast<uint8_t>((busIndex >> 24) & 0xFF);

	writeBuffer[9] = 0x14;

	gamepads[userIndex].toBytes(writeBuffer.data(), 10);

	const int flags = driverVersion[0] < 17 ? 0x2A400C : 0x2AE010;
	DWORD bytesReturned = 0;

	bool result = DeviceIoControl(handle.nativeHandle, flags,
	                              writeBuffer.data(), static_cast<DWORD>(writeBuffer.size()),
	                              readBuffer.data(), static_cast<DWORD>(readBuffer.size()),
	                              &bytesReturned, nullptr);

	if (!result || bytesReturned == 0)
	{
		return VBusStatus::IoctlRequestFailed;
	}

	//bVibrate    = readBuffer[1] == 0x08;
	//bLargeMotor = readBuffer[3];
	//bSmallMotor = readBuffer[4];
	//bLed        = readBuffer[8];

	if (driverVersion[0] >= 17 || readBuffer[1] == 0x08)
	{
		vibration[userIndex].leftMotor  = readBuffer[3];
		vibration[userIndex].rightMotor = readBuffer[4];
		leds[userIndex]                 = readBuffer[8];
	}

	if (driverVersion[0] < 17)
	{
		return VBusStatus::Success;
	}

	return readBuffer[9] == 0 ? VBusStatus::DeviceNotReady : VBusStatus::Success;
}

XInputGamepad ScpDevice::getGamepad(int userIndex)
{
	return gamepads[userIndex];
}

void ScpDevice::getVibration(int userIndex, uint8_t& leftMotor, uint8_t& rightMotor)
{
	const ScpVibration& vib = vibration[userIndex];

	leftMotor  = vib.leftMotor;
	rightMotor = vib.rightMotor;
}

uint8_t ScpDevice::getLed(int userIndex)
{
	return leds[userIndex];
}
