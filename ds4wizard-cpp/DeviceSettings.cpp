#include "stdafx.h"
#include "DeviceSettings.h"

DeviceSettings::DeviceSettings()
	: idle(DeviceIdleOptions::defaultIdleOptions)
{
	useProfileLight = false;
	useProfileIdle  = false;
}

bool DeviceSettings::operator==(const DeviceSettings& other) const
{
	return DeviceSettingsCommon::operator==(other) &&
	       name == other.name &&
	       profile == other.profile &&
	       useProfileLight == other.useProfileLight &&
	       useProfileIdle == other.useProfileIdle &&
	       idle == other.idle;
}

bool DeviceSettings::operator!=(const DeviceSettings& other) const
{
	return !(*this == other);
}

void DeviceSettings::readJson(const nlohmann::json& json)
{
	DeviceSettingsCommon::readJson(json);

	name            = json["name"];
	profile         = json.value("profile", "");
	useProfileLight = json["useProfileLight"];
	useProfileIdle  = json["useProfileIdle"];
	idle            = fromJson<DeviceIdleOptions>(json["idle"]);
}

void DeviceSettings::writeJson(nlohmann::json& json) const
{
	DeviceSettingsCommon::writeJson(json);

	json["name"]            = name;
	json["profile"]         = profile;
	json["useProfileLight"] = useProfileLight;
	json["useProfileIdle"]  = useProfileIdle;
	json["idle"]            = idle.toJson();
}
