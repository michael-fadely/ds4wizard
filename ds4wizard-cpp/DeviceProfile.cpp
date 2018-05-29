#include "stdafx.h"
#include "DeviceProfile.h"

static const char invalidCharacters[] = {
	'\\',
	'/',
	':',
	'*',
	'?',
	'"',
	'<',
	'>',
	'|'
};

std::string DeviceProfile::Name() const
{
	return name_;
}

void DeviceProfile::Name(const std::string& value)
{
	name_ = value;
	FileName = value + ".json";

	// TODO
	/*for (char c : invalidCharacters)
	{
		FileName = FileName.Replace(c, '_');
	}*/
}

DeviceProfile::DeviceProfile(const DeviceProfile& other) : DeviceSettingsCommon(other)
{
	Name(other.Name());
	ExclusiveMode   = other.ExclusiveMode;
	UseXInput       = other.UseXInput;
	AutoXInputIndex = other.AutoXInputIndex;
	XInputIndex     = other.XInputIndex;
	Idle            = other.Idle;

	Bindings = other.Bindings;
	Modifiers = other.Modifiers;

	if (!other.TouchRegions.empty())
	{
		for (auto& pair : other.TouchRegions)
		{
			TouchRegions[pair.first] = pair.second;
		}
	}
}

bool DeviceProfile::operator==(const DeviceProfile& other) const
{
	// TODO
	return /*Name.Equals(other.Name, StringComparison.InvariantCultureIgnoreCase)
		&&*/ ExclusiveMode == other.ExclusiveMode
		&& UseXInput == other.UseXInput
		&& AutoXInputIndex == other.AutoXInputIndex
		&& XInputIndex == other.XInputIndex
		&& TouchRegions == other.TouchRegions
		&& Bindings == other.Bindings
		&& Modifiers == other.Modifiers;
}