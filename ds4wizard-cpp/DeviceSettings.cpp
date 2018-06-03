#include "stdafx.h"
#include "DeviceSettings.h"

DeviceSettings::DeviceSettings()
{
	UseProfileLight = false;
	UseProfileIdle  = false;
	// TODO
	//Idle = DeviceIdleOptions::Default;
}

DeviceSettings::DeviceSettings(const DeviceSettings& s)
	: DeviceSettingsCommon(s)
{
	Name            = s.Name;
	Profile         = s.Profile;
	UseProfileLight = s.UseProfileLight;
	UseProfileIdle  = s.UseProfileIdle;
	Idle            = s.Idle;
}

bool DeviceSettings::operator==(const DeviceSettings& other) const
{
	return DeviceSettingsCommon::operator==(other) &&
	       Name == other.Name &&
	       Profile == other.Profile &&
	       UseProfileLight == other.UseProfileLight &&
	       UseProfileIdle == other.UseProfileIdle &&
	       Idle == other.Idle;
}

void DeviceSettings::readJson(const QJsonObject& json)
{
	// TODO DeviceSettingsCommon
	Name            = json["name"].toString().toStdString();
	Profile         = json["profile"].toString().toStdString();
	UseProfileLight = json["useProfileLight"].toBool();
	UseProfileIdle  = json["useProfileIdle"].toBool();
	Idle            = fromJson<DeviceIdleOptions>(json["idle"].toObject());
}

void DeviceSettings::writeJson(QJsonObject& json) const
{
	// TODO DeviceSettingsCommon
	json["name"]            = QString::fromStdString(Name);
	json["profile"]         = QString::fromStdString(Profile);
	json["useProfileLight"] = UseProfileLight;
	json["useProfileIdle"]  = UseProfileIdle;
	json["idle"]            = Idle.toJson();
}
