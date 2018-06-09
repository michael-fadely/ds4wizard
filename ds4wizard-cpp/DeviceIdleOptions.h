#pragma once

#include <chrono>
#include "JsonData.h"
#include <enum.h>

BETTER_ENUM(TimeUnit, int, seconds = 0, minutes = 1, hours = 2)

struct DeviceIdleOptions : JsonData
{
	using clock = std::chrono::high_resolution_clock;
	static const DeviceIdleOptions defaultIdleOptions;

	std::chrono::nanoseconds timeout = std::chrono::minutes(5);
	bool disconnect = true;
	TimeUnit unit = TimeUnit::minutes;

	DeviceIdleOptions() = default;
	DeviceIdleOptions(std::chrono::nanoseconds timeout, bool disconnect, TimeUnit unit);
	DeviceIdleOptions(const DeviceIdleOptions& other);

	bool operator==(const DeviceIdleOptions& other) const;
	bool operator!=(const DeviceIdleOptions& other) const;
	void readJson(const QJsonObject& json) override;
	void writeJson(QJsonObject& json) const override;
};
