#pragma once

#include <chrono>

enum class TimeUnit
{
	Seconds,
	Minutes,
	Hours
};

struct DeviceIdleOptions
{
	using clock = std::chrono::high_resolution_clock;
	/*[JsonIgnore]*/ clock::duration Timeout;

	bool Disconnect;

	TimeUnit Unit;

	DeviceIdleOptions() = default;

	DeviceIdleOptions(const DeviceIdleOptions& other)
	{
		Disconnect = other.Disconnect;
		Unit       = other.Unit;
		Timeout    = other.Timeout;
	}

	// TODO
	/*static const DeviceIdleOptions Default = DeviceIdleOptions()
	{
		Disconnect = true,
		Unit       = TimeUnit.Minutes,
		Timeout    = TimeSpan.FromMinutes(5.0)
	};*/

	bool operator==(const DeviceIdleOptions& other) const
	{
		return Disconnect == other.Disconnect && Timeout == other.Timeout;
	}
};
