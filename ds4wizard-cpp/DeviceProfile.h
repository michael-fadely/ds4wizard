#pragma once

#include <string>
#include <unordered_map>
#include <list>

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

	std::list<InputMap>      bindings;
	std::list<InputModifier> modifiers;

	DeviceProfile() = default;

	DeviceProfile(const DeviceProfile& other);

	bool operator==(const DeviceProfile& other) const;

	void readJson(const QJsonObject& json) override;
	void writeJson(QJsonObject& json) const override;
	static DeviceProfile defaultProfile();
};
