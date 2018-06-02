#pragma once

enum class ConnectionType
{
	usb,
	bluetooth
};

std::string toString(ConnectionType value);
ConnectionType ConnectionType_fromString(const std::string& value);
ConnectionType ConnectionType_fromQString(const QString& value);
