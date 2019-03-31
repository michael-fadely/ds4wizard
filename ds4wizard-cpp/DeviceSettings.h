#pragma once

#include <chrono>
#include <string>
#include "DeviceSettingsCommon.h"

/**
 * \brief Represents device-specific configuration for a \c Ds4Device.
 * \sa Ds4Device
 */
class DeviceSettings : public DeviceSettingsCommon
{
public:
	/**
	 * \brief User-configured name for the device.
	 */
	std::string name;

	/**
	 * \brief User-configured profile applied to the device. If none, empty string.
	 */
	std::string profile;

	/**
	 * \brief If \c true, light settings from the profile specified by \c profile
	 * will be used instead of those configured in \c DeviceSettingsCommon.
	 * \sa DeviceSettingsCommon
	 */
	bool useProfileLight = false;

	/**
	 * \brief If \c true, idle settings from the profile specified by \c profile
	 * will be used instead of those configured in \c DeviceSettingsCommon.
	 * \sa DeviceSettingsCommon
	 */
	bool useProfileIdle = false;

	/**
	 * \brief Threshold for latency at which the program will notify the user or begin mitigation.
	 */
	std::chrono::milliseconds latencyThreshold;

	DeviceSettings();
	DeviceSettings(const DeviceSettings& s) = default;

	bool operator==(const DeviceSettings& other) const;
	bool operator!=(const DeviceSettings& other) const;

	void readJson(const nlohmann::json& json) override;
	void writeJson(nlohmann::json& json) const override;
};
