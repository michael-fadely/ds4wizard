#include "stdafx.h"
#include "DeviceSettings.h"

using namespace std::chrono;

DeviceSettings::DeviceSettings()
	: useProfileLight(false),
	  useProfileIdle(false),
	  latencyThreshold(25)
{
}

bool DeviceSettings::operator==(const DeviceSettings& other) const
{
	return DeviceSettingsCommon::operator==(other) &&
	       name == other.name &&
	       profile == other.profile &&
	       useProfileLight == other.useProfileLight &&
	       useProfileIdle == other.useProfileIdle;
}

bool DeviceSettings::operator!=(const DeviceSettings& other) const
{
	return !(*this == other);
}

void DeviceSettings::readJson(const nlohmann::json& json)
{
	DeviceSettingsCommon::readJson(json);

	name             = json["name"];
	profile          = json.value("profile", "");
	useProfileLight  = json["useProfileLight"];
	useProfileIdle   = json["useProfileIdle"];
	latencyThreshold = milliseconds(json.value<uint64_t>("latencyThreshold", latencyThreshold.count()));
}

void DeviceSettings::writeJson(nlohmann::json& json) const
{
	DeviceSettingsCommon::writeJson(json);

	json["name"]             = name;
	json["profile"]          = profile;
	json["useProfileLight"]  = useProfileLight;
	json["useProfileIdle"]   = useProfileIdle;
	json["latencyThreshold"] = latencyThreshold.count();
}
