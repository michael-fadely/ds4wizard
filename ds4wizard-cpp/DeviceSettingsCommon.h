#pragma once

#include <cstdint>

#include "Ds4LightOptions.h"
#include "DeviceIdleOptions.h"

class Ds4Device;

class DeviceSettingsCommon : public JsonData
{
	bool notifiedLow;
	bool notifiedCharged;

public:
	Ds4LightOptions light;
	DeviceIdleOptions idle;
	bool notifyFullyCharged;
	uint8_t notifyBatteryLow;

	DeviceSettingsCommon();
	DeviceSettingsCommon(const DeviceSettingsCommon&) = default;
	DeviceSettingsCommon& operator=(const DeviceSettingsCommon&) = default;

	void displayNotifications(Ds4Device* device);

	bool operator==(const DeviceSettingsCommon& other) const;
	bool operator!=(const DeviceSettingsCommon& other) const;

	virtual void readJson(const nlohmann::json& json) override;
	virtual void writeJson(nlohmann::json& json) const override;
};
