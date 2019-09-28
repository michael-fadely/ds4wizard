#pragma once

#include <chrono>
#include "JsonData.h"

/**
 * \brief Configuration for handling device idle states.
 */
struct DeviceIdleOptions : JsonData
{
	using clock = std::chrono::high_resolution_clock;

	/**
	 * \brief Default idle state configuration.
	 */
	static const DeviceIdleOptions defaultIdleOptions;

	/**
	 * \brief Time threshold for taking action on idle state.
	 */
	std::chrono::seconds timeout = std::chrono::minutes(5);

	/**
	 * \brief If \c true, wireless devices will be disconnected upon idle detection.
	 */
	bool disconnect = true;

	DeviceIdleOptions() = default;

	/**
	 * \brief Constructs with essential values provided.
	 * \param timeout Time threshold for taking action on idle state.
	 * \param disconnect Disconnect wireless devices on idle.
	 */
	DeviceIdleOptions(std::chrono::seconds timeout, bool disconnect);
	
	DeviceIdleOptions(const DeviceIdleOptions& other) = default;

	DeviceIdleOptions& operator=(const DeviceIdleOptions& rhs) = default;

	bool operator==(const DeviceIdleOptions& other) const;
	bool operator!=(const DeviceIdleOptions& other) const;
	void readJson(const nlohmann::json& json) override;
	void writeJson(nlohmann::json& json) const override;
};
