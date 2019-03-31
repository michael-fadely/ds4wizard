#pragma once

#include <cstdint>

#include "Ds4LightOptions.h"
#include "DeviceIdleOptions.h"

class Ds4Device;

/**
 * \brief Represents common device configuration for a \c Ds4Device.
 * \sa Ds4Device
 */
class DeviceSettingsCommon : public JsonData
{
	bool notifiedLow;
	bool notifiedCharged;

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

	/**
	 * \brief Displays a notification, if configured.
	 * \param device The device that raised the notification.
	 */
	void displayNotifications(Ds4Device* device);

	bool operator==(const DeviceSettingsCommon& other) const;
	bool operator!=(const DeviceSettingsCommon& other) const;

	virtual void readJson(const nlohmann::json& json) override;
	virtual void writeJson(nlohmann::json& json) const override;
};
