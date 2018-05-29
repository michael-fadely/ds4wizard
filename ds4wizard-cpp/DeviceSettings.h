#pragma once

#include <string>
#include "DeviceSettingsCommon.h"

class DeviceSettings : DeviceSettingsCommon
{
public:
	std::string Name;
	std::string Profile;
	bool   UseProfileLight;
	bool   UseProfileIdle;

	DeviceSettings()
	{
		UseProfileLight = false;
		UseProfileIdle = false;
		// TODO
		//Idle = DeviceIdleOptions::Default;
	}

	DeviceSettings(const DeviceSettings& s) : DeviceSettingsCommon(s)
	{
		Name = s.Name;
		Profile = s.Profile;
		UseProfileLight = s.UseProfileLight;
		UseProfileIdle = s.UseProfileIdle;
	}

	bool operator==(const DeviceSettings& other)
	{
		return Name == other.Name && Profile == other.Profile
			&& UseProfileLight == other.UseProfileLight
			&& UseProfileIdle == other.UseProfileIdle;
	}
};
