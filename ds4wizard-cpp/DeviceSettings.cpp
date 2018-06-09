#include "stdafx.h"
#include "DeviceSettings.h"

DeviceSettings::DeviceSettings()
{
	useProfileLight = false;
	useProfileIdle  = false;
	idle            = DeviceIdleOptions::defaultIdleOptions;
}

DeviceSettings::DeviceSettings(const DeviceSettings& s)
	: DeviceSettingsCommon(s)
{
	name            = s.name;
	profile         = s.profile;
	useProfileLight = s.useProfileLight;
	useProfileIdle  = s.useProfileIdle;
	idle            = s.idle;
}

bool DeviceSettings::operator==(const DeviceSettings& other) const
{
	return DeviceSettingsCommon::operator==(other) &&
	       name == other.name &&
	       profile == other.profile &&
	       useProfileLight == other.useProfileLight &&
	       useProfileIdle == other.useProfileIdle &&
	       idle == other.idle;
}

bool DeviceSettings::operator!=(const DeviceSettings& other) const
{
	return !(*this == other);
}

void DeviceSettings::readJson(const QJsonObject& json)
{
	DeviceSettingsCommon::readJson(json);

	name            = json["name"].toString().toStdString();
	profile         = json["profile"].toString().toStdString();
	useProfileLight = json["useProfileLight"].toBool();
	useProfileIdle  = json["useProfileIdle"].toBool();
	idle            = fromJson<DeviceIdleOptions>(json["idle"].toObject());
}

void DeviceSettings::writeJson(QJsonObject& json) const
{
	DeviceSettingsCommon::writeJson(json);

	json["name"]            = QString::fromStdString(name);
	json["profile"]         = QString::fromStdString(profile);
	json["useProfileLight"] = useProfileLight;
	json["useProfileIdle"]  = useProfileIdle;
	json["idle"]            = idle.toJson();
}
