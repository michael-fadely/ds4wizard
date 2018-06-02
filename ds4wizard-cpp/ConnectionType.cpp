#include "stdafx.h"
#include "ConnectionType.h"

std::string toString(ConnectionType value)
{
	switch (value)
	{
		case ConnectionType::usb:
			return "usb";
		case ConnectionType::bluetooth:
			return "bluetooth";
		default:
			throw std::out_of_range("invalid value for enum ConnectionType");
	}
}

ConnectionType ConnectionType_fromString(const std::string& value)
{
	if (value == "usb")
	{
		return ConnectionType::usb;
	}

	if (value == "bluetooth")
	{
		return ConnectionType::bluetooth;
	}

	throw std::out_of_range("invalid value for enum ConnectionType");
}

ConnectionType ConnectionType_fromQString(const QString& value)
{
	return ConnectionType_fromString(value.toStdString());
}
