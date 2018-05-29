#pragma once

#include <cstdint>

#include "Ds4LightOptions.h"
#include "DeviceIdleOptions.h"

class DeviceSettingsCommon
{
public:
	bool notifiedLow;
	bool notifiedCharged;

	Ds4LightOptions   Light;
	DeviceIdleOptions Idle;

	bool NotifyFullyCharged;
	uint8_t NotifyBatteryLow;

	DeviceSettingsCommon()
	{
		Light = {};
		Idle = {};

		notifiedLow = false;
		notifiedCharged = true;
		NotifyFullyCharged = true;
		NotifyBatteryLow = 2;
	}

	DeviceSettingsCommon(const DeviceSettingsCommon& other)
	{
		Light = other.Light;
		Idle = other.Idle;

		NotifyFullyCharged = other.NotifyFullyCharged;
		NotifyBatteryLow = other.NotifyBatteryLow;

		notifiedLow = other.notifiedLow;
		notifiedCharged = other.notifiedCharged;
	}

	/* TODO
	void DisplayNotifications(Ds4Device device)
	{
		if (NotifyBatteryLow > 0)
		{
			if (device.UsbConnected || device.Charging || device.Battery > NotifyBatteryLow)
			{
				notifiedLow = false;
			}
			else if (!notifiedLow)
			{
				notifiedLow = true;
				Logger.WriteLine(LogLevel.Warning, device.Name, string.Format(Resources.BatteryLow, device.Battery * 10));
			}
		}

		if (NotifyFullyCharged)
		{
			if (!device.UsbConnected || device.Battery < 10)
			{
				notifiedCharged = false;
			}
			else if (!notifiedCharged)
			{
				notifiedCharged = true;
				Logger.WriteLine(LogLevel.Info, device.Name, Resources.BatteryCharged);
			}
		}
	}
	*/

	bool operator==(const DeviceSettingsCommon& other) const
	{
		return Light == other.Light && Idle == other.Idle
			&& NotifyFullyCharged == other.NotifyFullyCharged
			&& NotifyBatteryLow == other.NotifyBatteryLow;
	}
};

