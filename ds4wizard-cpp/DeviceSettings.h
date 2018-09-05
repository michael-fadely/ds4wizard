#pragma once

#include <string>
#include "DeviceSettingsCommon.h"

class DeviceSettings : public DeviceSettingsCommon
{
public:
	std::string name;
	std::string profile;
	bool        useProfileLight = false;
	bool        useProfileIdle = false;

	DeviceSettings();
	DeviceSettings(const DeviceSettings& s) = default;

	bool operator==(const DeviceSettings& other) const;
	bool operator!=(const DeviceSettings& other) const;

	void readJson(const nlohmann::json& json) override;
	void writeJson(nlohmann::json& json) const override;
};
