#include "stdafx.h"
#include <windows.h>
#include "ScpDevice.h"
#include <SetupAPI.h>
#include "busenum.h"
#include "lock.h"

// {F679F562-3164-42CE-A4DB-E7DDBE723909}
extern const GUID GUID_DEVINTERFACE_SCPVBUS;

int ScpDevice::GetFreePort()
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

ScpDevice::ScpDevice(HANDLE handle)
{
	this->handle = handle;

	uint64_t version;
	if (GetDriverVersion(version) != 0)
	{
		return;
	}

	Version[0] = static_cast<short>(version >> 48);
	Version[1] = static_cast<short>((version >> 32) & 0xFFFF);
	Version[2] = static_cast<short>((version >> 16) & 0xFFFF);
	Version[3] = static_cast<short>(version & 0xFFFF);
}

int ScpDevice::GetDriverVersion(uint64_t& version)
{
	version = 0;

	HANDLE invalid = INVALID_HANDLE_VALUE;
	auto devInfoSet = SetupDiGetClassDevs(&GUID_DEVINTERFACE_SCPVBUS, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (devInfoSet == invalid)
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

	// DI_FLAGSEX_INSTALLEDDRIVER
	p.FlagsEx |= 0x04000000;

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

void ScpDevice::Close()
{
	if (handle != nullptr && handle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(handle);
		handle = nullptr;
	}
}

bool ScpDevice::Connect(int userIndex)
{
	if (userIndex < 0 || userIndex > 3)
	{
		throw /*new IndexOutOfRangeException("User index must be in the range 0 to 3.") // TODO */;
	}

	BUSENUM_UNPLUG_HARDWARE buffer = BUSENUM_UNPLUG_HARDWARE::Create(userIndex);

	int flags = Version[0] < 17 ? 0x2A4000 : 0x2AA004;

	bool result = DeviceIoControl(handle, flags,
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

bool ScpDevice::Disconnect(int userIndex, bool force)
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

	BUSENUM_UNPLUG_HARDWARE buffer = BUSENUM_UNPLUG_HARDWARE::Create(userIndex);

	if (Version[0] >= 17)
	{
		buffer.Flags = force ? 0x0001 : 0x0000;
	}

	int flags = Version[0] < 17 ? 0x2A4004 : 0x2AA008;

	return DeviceIoControl(handle, flags,
			&buffer, buffer.Size, nullptr, 0, nullptr, nullptr);
}

VBusStatus ScpDevice::SyncState(int userIndex, const XInputGamepad& gamepad)
{
	gamepads[userIndex] = gamepad;
	return SyncState(userIndex);
}

VBusStatus ScpDevice::SyncState(int userIndex)
{
	int busIndex = userIndex + 1;

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

	int flags = Version[0] < 17 ? 0x2A400C : 0x2AE010;
	DWORD bytesReturned = 0;

	bool result = DeviceIoControl(handle, flags,
	                              writeBuffer.data(), writeBuffer.size(), readBuffer.data(), readBuffer.size(), &bytesReturned, nullptr);

	if (!result || bytesReturned == 0)
	{
		return VBusStatus::IoctlRequestFailed;
	}

	//bVibrate    = readBuffer[1] == 0x08;
	//bLargeMotor = readBuffer[3];
	//bSmallMotor = readBuffer[4];
	//bLed        = readBuffer[8];

	if (Version[0] >= 17 || readBuffer[1] == 0x08)
	{
		vibration[userIndex].LeftMotor  = readBuffer[3];
		vibration[userIndex].RightMotor = readBuffer[4];
		leds[userIndex]                 = readBuffer[8];
	}

	if (Version[0] < 17)
	{
		return VBusStatus::Success;
	}

	return readBuffer[9] == 0 ? VBusStatus::DeviceNotReady : VBusStatus::Success;
}

XInputGamepad ScpDevice::GetGamepad(int userIndex)
{
	return gamepads[userIndex];
}

void ScpDevice::GetVibration(int userIndex, uint8_t& leftMotor, uint8_t& rightMotor)
{
	ScpVibration vib = vibration[userIndex];
	leftMotor        = vib.LeftMotor;
	rightMotor       = vib.RightMotor;
}

uint8_t ScpDevice::GetLed(int userIndex)
{
	return leds[userIndex];
}
