#include "stdafx.h"
#include "Settings.h"

bool Settings::operator==(const Settings& rhs) const
{
	return preferredConnection == rhs.preferredConnection &&
	       startMinimized      == rhs.startMinimized &&
	       minimizeToTray      == rhs.minimizeToTray;
}

bool Settings::operator!=(const Settings& rhs) const
{
	return !(*this == rhs);
}

void Settings::readJson(const QJsonObject& json)
{
	if (json.contains("preferredConnection"))
	{
		preferredConnection = ConnectionType::_from_string(json["preferredConnection"].toString().toStdString().c_str());
	}

	if (json.contains("startMinimized"))
	{
		startMinimized = json["startMinimized"].toBool();
	}

	if (json.contains("minimizeToTray"))
	{
		minimizeToTray = json["minimizeToTray"].toBool();
	}
}

void Settings::writeJson(QJsonObject& json) const
{
	json["preferredConnection"] = preferredConnection._to_string();
	json["startMinimized"]      = startMinimized;
	json["minimizeToTray"]      = minimizeToTray;
}
