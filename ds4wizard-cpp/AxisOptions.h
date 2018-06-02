#pragma once

#include "enums.h"

class AxisOptions
{
public:
	float/* ? */ Multiplier = 1.0f;
	AxisPolarity/* ?*/ Polarity = AxisPolarity::Positive;

	AxisOptions() = default;

	AxisOptions(AxisPolarity polarity)
	{
		Polarity = polarity;
	}

	AxisOptions(const AxisOptions& other)
	{
		Multiplier = other.Multiplier;
		Polarity = other.Polarity;
	}

	bool operator==(const AxisOptions& other) const
	{
		return Multiplier == other.Multiplier && Polarity == other.Polarity;
	}
};

class InputAxisOptions : public AxisOptions
{
public:
	bool Invert;
	DeadZoneMode DeadZoneMode = DeadZoneMode::Scale;
	float DeadZone = 0.0f;

	InputAxisOptions() = default;

	InputAxisOptions(AxisPolarity polarity) : AxisOptions(polarity)
	{
	}

	InputAxisOptions(const InputAxisOptions& other) : AxisOptions(other)
	{
		Invert       = other.Invert;
		DeadZoneMode = other.DeadZoneMode;
		DeadZone     = other.DeadZone;
	}

	bool operator==(const InputAxisOptions& other) const
	{
		return AxisOptions::operator==(other)
			&& Invert == other.Invert
			&& DeadZoneMode == other.DeadZoneMode
			&& DeadZone == other.DeadZone;
	}

	void ApplyDeadZone(float& analog) const
	{
		switch (DeadZoneMode::Scale)
		{
			case DeadZoneMode::HardLimit:
				analog = analog >= DeadZone ? analog : 0.0f;
				break;

			case DeadZoneMode::Scale:
				analog = std::max(0.0f, (analog - DeadZone) / (1.0f - DeadZone));
				break;

			default:
				throw /*new ArgumentOutOfRangeException(nameof(DeadZoneMode), DeadZoneMode, "Invalid deadzone mode.")*/;
		}

		if (Invert == true)
		{
			analog = 1.0f - analog;
		}

		analog *= Multiplier;
	}
};