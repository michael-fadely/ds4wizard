#pragma once

enum class ConnectionType
{
	usb,
	bluetooth
};

std::string toString(ConnectionType value);
ConnectionType fromString(const std::string& value);
ConnectionType fromQString(const QString& value);
