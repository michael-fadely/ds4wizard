#pragma once

#include <chrono>
#include "JsonData.h"

enum class TimeUnit
{
	seconds,
	minutes,
	hours
};

std::string toString(TimeUnit value);
TimeUnit fromString(const std::string& value);
TimeUnit fromQString(const QString& value);

struct DeviceIdleOptions : JsonData
{
	using clock = std::chrono::high_resolution_clock;
	static const DeviceIdleOptions Default;

	clock::duration Timeout;
	bool Disconnect;
	TimeUnit Unit;

	DeviceIdleOptions() = default;
	DeviceIdleOptions(clock::duration timeout, bool disconnect, TimeUnit unit);
	DeviceIdleOptions(const DeviceIdleOptions& other);

	bool operator==(const DeviceIdleOptions& other) const;
	void readJson(const QJsonObject& json) override;
	void writeJson(QJsonObject& json) const override;
};
