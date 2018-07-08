#pragma once

#include <string>
#include <unordered_map>
#include <deque>

#include "DeviceSettingsCommon.h"
#include "Ds4TouchRegion.h"
#include "InputMap.h"

class DeviceProfile : public DeviceSettingsCommon
{
public:
	std::string fileName() const;
	std::string name;
	bool        exclusiveMode   = true;
	bool        useXInput       = true;
	bool        autoXInputIndex = true;
	int         xinputIndex     = 0;

	std::unordered_map<std::string, Ds4TouchRegion> touchRegions;

	std::deque<InputMap>      bindings;
	std::deque<InputModifier> modifiers;

	DeviceProfile() = default;

	DeviceProfile(const DeviceProfile& other);

	bool operator==(const DeviceProfile& other) const;

	void readJson(const nlohmann::json& json) override;
	void writeJson(nlohmann::json& json) const override;
	static DeviceProfile defaultProfile();
};
