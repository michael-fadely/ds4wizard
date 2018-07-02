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

void Settings::readJson(const nlohmann::json& json)
{
	if (json.find("preferredConnection") != json.end())
	{
		preferredConnection = ConnectionType::_from_string(json["preferredConnection"].get<std::string>().c_str());
	}

	if (json.find("startMinimized") != json.end())
	{
		startMinimized = json["startMinimized"];
	}

	if (json.find("minimizeToTray") != json.end())
	{
		minimizeToTray = json["minimizeToTray"];
	}
}

void Settings::writeJson(nlohmann::json& json) const
{
	json["preferredConnection"] = preferredConnection._to_string();
	json["startMinimized"]      = startMinimized;
	json["minimizeToTray"]      = minimizeToTray;
}
