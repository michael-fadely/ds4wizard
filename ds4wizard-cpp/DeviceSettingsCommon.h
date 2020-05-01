#pragma once

#include <cstdint>

#include "Ds4LightOptions.h"
#include "DeviceIdleOptions.h"

/**
 * \brief Represents common device configuration for a \c Ds4Device.
 * \sa Ds4Device
 */
class DeviceSettingsCommon : public JsonData
{
public:
	/** \brief Options for the light bar. */
	Ds4LightOptions light;

	/**
	 * \brief Options for handling idle state of a \c Ds4Device
	 * \sa Ds4Device
	 */
	DeviceIdleOptions idle;

	/**
	 * \brief If \c true, the program will display a notification when
	 * a \c Ds4Device reaches full charge.
	 */
	bool notifyFullyCharged;

	/**
	 * \brief Threshold for low charge level. When met, the program will
	 * display a notification.
	 */
	uint8_t notifyBatteryLow;

	DeviceSettingsCommon();
	DeviceSettingsCommon(const DeviceSettingsCommon&) = default;
	DeviceSettingsCommon& operator=(const DeviceSettingsCommon&) = default;

	bool operator==(const DeviceSettingsCommon& other) const;
	bool operator!=(const DeviceSettingsCommon& other) const;

	void readJson(const nlohmann::json& json) override;
	void writeJson(nlohmann::json& json) const override;
};
