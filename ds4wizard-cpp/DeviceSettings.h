#pragma once

#include <string>
#include "DeviceSettingsCommon.h"
#include "JsonData.h"

class DeviceSettings : public DeviceSettingsCommon, public JsonData
{
public:
	std::string       Name;
	std::string       Profile;
	bool              UseProfileLight;
	bool              UseProfileIdle;
	DeviceIdleOptions Idle;

	DeviceSettings();

	DeviceSettings(const DeviceSettings& s);

	bool operator==(const DeviceSettings& other) const;
	void readJson(const QJsonObject& json) override;
	void writeJson(QJsonObject& json) const override;
};
