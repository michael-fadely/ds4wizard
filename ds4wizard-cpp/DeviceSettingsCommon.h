#pragma once

#include <cstdint>

#include "Ds4LightOptions.h"
#include "DeviceIdleOptions.h"

class Ds4Device;

class DeviceSettingsCommon
{
	bool notifiedLow;
	bool notifiedCharged;

public:
	Ds4LightOptions   Light;
	DeviceIdleOptions Idle;

	bool NotifyFullyCharged;
	uint8_t NotifyBatteryLow;

	DeviceSettingsCommon();

	DeviceSettingsCommon(const DeviceSettingsCommon& other);

	void DisplayNotifications(Ds4Device* device);

	bool operator==(const DeviceSettingsCommon& other) const;
};

