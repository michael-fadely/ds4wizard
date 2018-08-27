#include "stdafx.h"
#include "DeviceIdleOptions.h"

const DeviceIdleOptions DeviceIdleOptions::defaultIdleOptions(std::chrono::minutes(5), true);

DeviceIdleOptions::DeviceIdleOptions(std::chrono::microseconds timeout, bool disconnect)
	: timeout(timeout),
	  disconnect(disconnect)
{
}

DeviceIdleOptions::DeviceIdleOptions(const DeviceIdleOptions& other)
	: timeout(other.timeout),
	  disconnect(other.disconnect)
{
}

bool DeviceIdleOptions::operator==(const DeviceIdleOptions& other) const
{
	return disconnect == other.disconnect && timeout == other.timeout;
}

bool DeviceIdleOptions::operator!=(const DeviceIdleOptions& other) const
{
	return !(*this == other);
}

void DeviceIdleOptions::readJson(const nlohmann::json& json)
{
	this->timeout    = std::chrono::microseconds(json["timeout"].get<int64_t>());
	this->disconnect = json["disconnect"];
}

void DeviceIdleOptions::writeJson(nlohmann::json& json) const
{
	json["timeout"]    = this->timeout.count();
	json["disconnect"] = this->disconnect;
}
