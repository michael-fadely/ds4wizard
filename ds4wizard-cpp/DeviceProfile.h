#pragma once

#include <string>
#include <unordered_map>
#include <list>

#include "DeviceSettingsCommon.h"
#include "Ds4TouchRegion.h"
#include "InputMap.h"

class DeviceProfile : public DeviceSettingsCommon, public JsonData
{
public:
	std::string FileName() const;
	std::string Name;
	bool        ExclusiveMode   = true;
	bool        UseXInput       = true;
	bool        AutoXInputIndex = true;
	int         XInputIndex     = 0;

	std::unordered_map<std::string, Ds4TouchRegion> TouchRegions;

	std::list<InputMap>      Bindings;
	std::list<InputModifier> Modifiers;

	DeviceProfile() = default;

	DeviceProfile(const DeviceProfile& other);

	bool operator==(const DeviceProfile& other) const;

	void readJson(const QJsonObject& json) override;
	void writeJson(QJsonObject& json) const override;
	// TODO
	static DeviceProfile Default();
};
