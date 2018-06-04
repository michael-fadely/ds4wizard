#include "stdafx.h"
#include "DeviceSettingsCommon.h"

DeviceSettingsCommon::DeviceSettingsCommon()
{
	Light = {};
	Idle  = {};

	notifiedLow        = false;
	notifiedCharged    = true;
	NotifyFullyCharged = true;
	NotifyBatteryLow   = 2;
}

DeviceSettingsCommon::DeviceSettingsCommon(const DeviceSettingsCommon& other)
{
	Light = other.Light;
	Idle  = other.Idle;

	NotifyFullyCharged = other.NotifyFullyCharged;
	NotifyBatteryLow   = other.NotifyBatteryLow;

	notifiedLow     = other.notifiedLow;
	notifiedCharged = other.notifiedCharged;
}

void DeviceSettingsCommon::DisplayNotifications(Ds4Device* device)
{
	/* TODO
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
	*/
}

bool DeviceSettingsCommon::operator==(const DeviceSettingsCommon& other) const
{
	return Light == other.Light && Idle == other.Idle
	       && NotifyFullyCharged == other.NotifyFullyCharged
	       && NotifyBatteryLow == other.NotifyBatteryLow;
}

bool DeviceSettingsCommon::operator!=(const DeviceSettingsCommon& other) const
{
	return !(*this == other);
}

void DeviceSettingsCommon::readJson(const QJsonObject& json)
{
}

void DeviceSettingsCommon::writeJson(QJsonObject& json) const
{
	json["light"] = Light.toJson();
	json["idle"] = Idle.toJson();
	json["notifyFullyCharged"] = NotifyFullyCharged;
	json["notifyBatteryLow"] = NotifyFullyCharged;
}
