#include "stdafx.h"
#include "DeviceIdleOptions.h"

const DeviceIdleOptions DeviceIdleOptions::defaultIdleOptions(std::chrono::seconds(5), true, TimeUnit::minutes);

DeviceIdleOptions::DeviceIdleOptions(std::chrono::nanoseconds timeout, bool disconnect, TimeUnit unit)
	: timeout(timeout),
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

void DeviceIdleOptions::readJson(const QJsonObject& json)
{
	this->timeout    = std::chrono::milliseconds(json["timeout"].toInt());
	this->disconnect = json["disconnect"].toBool();
	this->unit       = TimeUnit::_from_string(json["unit"].toString().toStdString().c_str()); // this is disgusting
}

void DeviceIdleOptions::writeJson(QJsonObject& json) const
{
	json["timeout"]    = this->timeout.count();
	json["disconnect"] = this->disconnect;
	json["unit"]       = this->unit._to_string();
}
