#include "stdafx.h"
#include "DeviceSettingsCommon.h"
#include "Logger.h"
#include <sstream>

DeviceSettingsCommon::DeviceSettingsCommon()
	: notifiedLow(false),
	  notifiedCharged(true),
	  idle({}),
	  notifyFullyCharged(true),
	  notifyBatteryLow(2)
{
}

DeviceSettingsCommon::DeviceSettingsCommon(const DeviceSettingsCommon& other)
	: notifiedLow(other.notifiedLow),
	  notifiedCharged(other.notifiedCharged),
	  light(other.light),
	  idle(other.idle),
	  notifyFullyCharged(other.notifyFullyCharged),
	  notifyBatteryLow(other.notifyBatteryLow)
{
}

void DeviceSettingsCommon::displayNotifications(Ds4Device* device)
{
	if (notifyBatteryLow > 0)
	{
		if (device->usbConnected() || device->charging() || device->battery() > notifyBatteryLow)
		{
			notifiedLow = false;
		}
		else if (!notifiedLow)
		{
			notifiedLow = true;

			// TODO: translatable
			std::stringstream message;
			message << "Battery running low! (" << device->battery() * 10 << ")";

			Logger::writeLine(LogLevel::warning, device->name(), message.str());
		}
	}

	if (notifyFullyCharged)
	{
		if (!device->usbConnected() || device->battery() < 10)
		{
			notifiedCharged = false;
		}
		else if (!notifiedCharged)
		{
			notifiedCharged = true;
			// TODO: translatable
			Logger::writeLine(LogLevel::info, device->name(), "Fully charged.");
		}
	}
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

void DeviceSettingsCommon::readJson(const nlohmann::json& json)
{
	light              = fromJson<Ds4LightOptions>(json["light"]);
	idle               = fromJson<DeviceIdleOptions>(json["idle"]);
	notifyFullyCharged = json.value("notifyFullyCharged", true);
	notifyBatteryLow   = static_cast<uint8_t>(json.value("notifyBatteryLow", 2));
}

void DeviceSettingsCommon::writeJson(nlohmann::json& json) const
{
	json["light"]              = light.toJson();
	json["idle"]               = idle.toJson();
	json["notifyFullyCharged"] = notifyFullyCharged;
	json["notifyBatteryLow"]   = notifyBatteryLow;
}
