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
		preferredConnection = fromQString(json["preferredConnection"].toString());
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
	json["preferredConnection"] = QString::fromStdString(toString(preferredConnection));
	json["startMinimized"]      = startMinimized;
	json["minimizeToTray"]      = minimizeToTray;
}
