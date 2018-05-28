#pragma once
#include "ConnectionType.h"
#include "JsonData.h"

struct Settings : JsonData
{
	Settings() = default;
	Settings(Settings&) = default;

	ConnectionType preferredConnection = ConnectionType::usb;
	bool startMinimized = false;
	bool minimizeToTray = true;

	Settings& operator=(const Settings& rhs) = default;
	bool operator==(const Settings& rhs) const;
	bool operator!=(const Settings& rhs) const;

	void read(const QJsonObject& json) override;
	void write(QJsonObject& json) override;
};
