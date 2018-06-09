#pragma once

#include <string>
#include "DeviceSettingsCommon.h"

class DeviceSettings : public DeviceSettingsCommon
{
public:
	std::string       name;
	std::string       profile;
	bool              useProfileLight;
	bool              useProfileIdle;
	DeviceIdleOptions idle;

	DeviceSettings();

	DeviceSettings(const DeviceSettings& s);

	bool operator==(const DeviceSettings& other) const;
	bool operator!=(const DeviceSettings& other) const;

	void readJson(const QJsonObject& json) override;
	void writeJson(QJsonObject& json) const override;
};
