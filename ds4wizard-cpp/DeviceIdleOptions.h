#pragma once

#include <chrono>
#include "JsonData.h"
#include <enum.h>

struct DeviceIdleOptions : JsonData
{
	using clock = std::chrono::high_resolution_clock;
	static const DeviceIdleOptions defaultIdleOptions;

	std::chrono::microseconds timeout = std::chrono::minutes(5);
	bool disconnect = true;

	DeviceIdleOptions() = default;
	DeviceIdleOptions(std::chrono::microseconds timeout, bool disconnect);
	DeviceIdleOptions(const DeviceIdleOptions& other);

	bool operator==(const DeviceIdleOptions& other) const;
	bool operator!=(const DeviceIdleOptions& other) const;
	void readJson(const nlohmann::json& json) override;
	void writeJson(nlohmann::json& json) const override;
};
