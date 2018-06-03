#include "stdafx.h"
#include "Ds4LightOptions.h"

Ds4LightOptions::Ds4LightOptions(const Ds4Color& color)
{
	Color = color;
}

Ds4LightOptions::Ds4LightOptions(const Ds4LightOptions& other)
{
	Color           = other.Color;
	IdleFade        = other.IdleFade;
	LowBatteryColor = other.LowBatteryColor;
}

// TODO: replace with IComparable (if this were C#)
bool Ds4LightOptions::operator==(const Ds4LightOptions& other) const
{
	bool result = AutomaticColor == other.AutomaticColor &&
	              LowBatteryColor == other.LowBatteryColor;

	if (!AutomaticColor || !other.AutomaticColor)
	{
		result = result && Color == other.Color;
	}

	return result;
}

bool Ds4LightOptions::operator!=(const Ds4LightOptions& other) const
{
	return !(*this == other);
}
