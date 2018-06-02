#include "stdafx.h"
#include "DeviceIdleOptions.h"

std::string toString(TimeUnit value)
{
	switch (value)
	{
		case TimeUnit::hours:
			return "hours";
		case TimeUnit::minutes:
			return "minutes";
		case TimeUnit::seconds:
			return "seconds";
		default:
			throw std::out_of_range("invalid value for enum TimeUnit");
	}
}

TimeUnit fromString(const std::string& value)
{
	if (value == "hours")
	{
		return TimeUnit::hours;
	}

	if (value == "minutes")
	{
		return TimeUnit::minutes;
	}

	if (value == "seconds")
	{
		return TimeUnit::seconds;
	}

	throw std::out_of_range("invalid value for enum TimeUnit");
}

TimeUnit fromQString(const QString& value)
{
	return fromString(value.toStdString());
}

const DeviceIdleOptions DeviceIdleOptions::Default(std::chrono::seconds(5), true, TimeUnit::minutes);

DeviceIdleOptions::DeviceIdleOptions(clock::duration timeout, bool disconnect, TimeUnit unit)
{
	this->Timeout    = timeout;
	this->Disconnect = disconnect;
	this->Unit       = unit;
}

DeviceIdleOptions::DeviceIdleOptions(const DeviceIdleOptions& other)
{
	Timeout    = other.Timeout;
	Disconnect = other.Disconnect;
	Unit       = other.Unit;
}

bool DeviceIdleOptions::operator==(const DeviceIdleOptions& other) const
{
	return Disconnect == other.Disconnect && Timeout == other.Timeout;
}

void DeviceIdleOptions::readJson(const QJsonObject& json)
{
	this->Timeout    = std::chrono::milliseconds(json["timeout"].toInt());
	this->Disconnect = json["disconnect"].toBool();
	this->Unit       = fromQString(json["unit"].toString());
}

void DeviceIdleOptions::writeJson(QJsonObject& json) const
{
	json["timeout"]    = this->Timeout.count();
	json["disconnect"] = this->Disconnect;
	json["unit"]       = QString::fromStdString(toString(this->Unit));
}
