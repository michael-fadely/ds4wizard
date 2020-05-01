#include "pch.h"
#include "DeviceSettingsCommon.h"

DeviceSettingsCommon::DeviceSettingsCommon()
	: idle({}),
	  notifyFullyCharged(true),
	  notifyBatteryLow(2)
{
}

bool DeviceSettingsCommon::operator==(const DeviceSettingsCommon& other) const
{
	return light == other.light &&
	       idle == other.idle &&
	       notifyFullyCharged == other.notifyFullyCharged &&
	       notifyBatteryLow == other.notifyBatteryLow;
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
