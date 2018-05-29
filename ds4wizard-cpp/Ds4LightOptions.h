#pragma once

#include "Ds4Color.h"

class Ds4LightOptions
{
public:
	bool     AutomaticColor = true;
	Ds4Color Color;
	bool     IdleFade = true;
	Ds4Color LowBatteryColor;

	Ds4LightOptions(const Ds4Color& color)
	{
		Color = color;
	}

	Ds4LightOptions(const Ds4LightOptions& other)
	{
		Color = other.Color;
		IdleFade = other.IdleFade;
		LowBatteryColor = other.LowBatteryColor;
	}

	Ds4LightOptions() = default;

	// TODO: replace with IComparable (if this were C#)
	bool operator==(const Ds4LightOptions& other) const
	{
		bool result = AutomaticColor == other.AutomaticColor &&
			LowBatteryColor == other.LowBatteryColor;

		if (!AutomaticColor || !other.AutomaticColor)
		{
			result = result && Color == other.Color;
		}

		return result;
	}
};
