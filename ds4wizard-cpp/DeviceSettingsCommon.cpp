#include "stdafx.h"
#include "DeviceSettingsCommon.h"

DeviceSettingsCommon::DeviceSettingsCommon()
{
	light              = {};
	idle               = {};
	notifyFullyCharged = true;
	notifyBatteryLow   = 2;

	notifiedLow     = false;
	notifiedCharged = true;
}

DeviceSettingsCommon::DeviceSettingsCommon(const DeviceSettingsCommon& other)
{
	light = other.light;
	idle  = other.idle;

	notifyFullyCharged = other.notifyFullyCharged;
	notifyBatteryLow   = other.notifyBatteryLow;

	notifiedLow     = other.notifiedLow;
	notifiedCharged = other.notifiedCharged;
}

void DeviceSettingsCommon::displayNotifications(Ds4Device* device)
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
	return light == other.light && idle == other.idle
	       && notifyFullyCharged == other.notifyFullyCharged
	       && notifyBatteryLow == other.notifyBatteryLow;
}

bool DeviceSettingsCommon::operator!=(const DeviceSettingsCommon& other) const
{
	return !(*this == other);
}

void DeviceSettingsCommon::readJson(const QJsonObject& json)
{
	light              = fromJson<Ds4LightOptions>(json["light"].toObject());
	idle               = fromJson<DeviceIdleOptions>(json["idle"].toObject());
	notifyFullyCharged = json["notifyFullyCharged"].toBool(true);
	notifyBatteryLow   = static_cast<uint8_t>(json["notifyBatteryLow"].toInt(2));
}

void DeviceSettingsCommon::writeJson(QJsonObject& json) const
{
	json["light"]              = light.toJson();
	json["idle"]               = idle.toJson();
	json["notifyFullyCharged"] = notifyFullyCharged;
	json["notifyBatteryLow"]   = notifyBatteryLow;
}
