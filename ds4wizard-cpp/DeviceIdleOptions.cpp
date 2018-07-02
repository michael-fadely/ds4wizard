#include "stdafx.h"
#include "DeviceIdleOptions.h"

const DeviceIdleOptions DeviceIdleOptions::defaultIdleOptions(std::chrono::minutes(5), true, TimeUnit::minutes);

DeviceIdleOptions::DeviceIdleOptions(std::chrono::milliseconds timeout, bool disconnect, TimeUnit unit)
	: timeout(std::move(timeout)),
	  disconnect(disconnect),
	  unit(unit)
{
}

DeviceIdleOptions::DeviceIdleOptions(const DeviceIdleOptions& other)
	: timeout(other.timeout),
	  disconnect(other.disconnect),
	  unit(other.unit)
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
	this->timeout    = std::chrono::milliseconds(json["timeout"].get<int64_t>());
	this->disconnect = json["disconnect"];
	this->unit       = TimeUnit::_from_string(json["unit"].get<std::string>().c_str());;
}

void DeviceIdleOptions::writeJson(nlohmann::json& json) const
{
	json["timeout"]    = this->timeout.count();
	json["disconnect"] = this->disconnect;
	json["unit"]       = this->unit._to_string();
}
