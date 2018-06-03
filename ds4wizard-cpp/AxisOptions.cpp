#include "stdafx.h"
#include "AxisOptions.h"

AxisOptions::AxisOptions(AxisPolarity polarity)
{
	Polarity = polarity;
}

AxisOptions::AxisOptions(const AxisOptions& other)
{
	Multiplier = other.Multiplier;
	Polarity   = other.Polarity;
}

bool AxisOptions::operator==(const AxisOptions& other) const
{
	return Multiplier == other.Multiplier && Polarity == other.Polarity;
}

bool AxisOptions::operator!=(const AxisOptions& other) const
{
	return !(*this == other);
}

InputAxisOptions::InputAxisOptions(AxisPolarity polarity)
	: AxisOptions(polarity)
{
}

InputAxisOptions::InputAxisOptions(const InputAxisOptions& other)
	: AxisOptions(other)
{
	Invert       = other.Invert;
	DeadZoneMode = other.DeadZoneMode;
	DeadZone     = other.DeadZone;
}

void InputAxisOptions::ApplyDeadZone(float& analog) const
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

bool InputAxisOptions::operator==(const InputAxisOptions& other) const
{
	return AxisOptions::operator==(other)
	       && Invert == other.Invert
	       && DeadZoneMode == other.DeadZoneMode
	       && DeadZone == other.DeadZone;
}

bool InputAxisOptions::operator!=(const InputAxisOptions& other) const
{
	return !(*this == other);
}

XInputAxes::XInputAxes(const XInputAxes& other)
{
	Axes = other.Axes;

	if (!other.Options.empty())
	{
		for (auto& pair : other.Options)
		{
			Options[pair.first] = pair.second;
		}
	}
}

AxisOptions XInputAxes::GetAxisOptions(XInputAxis::T axis)
{
	if (Options.empty())
	{
		return AxisOptions();
	}

	const auto it = Options.find(axis);

	if (it != Options.end())
	{
		return it->second;
	}

	// HACK: separate into its own cache
	AxisOptions result {};
	Options[axis] = result;
	return result;
}

bool XInputAxes::operator==(const XInputAxes& other) const
{
	return Axes == other.Axes && Options == other.Options;
}

bool XInputAxes::operator!=(const XInputAxes& other) const
{
	return !(*this == other);
}

MouseAxes::MouseAxes(const MouseAxes& other)
{
	Directions = other.Directions;

	if (!other.Options.empty())
	{
		for (auto& pair : other.Options)
		{
			Options[pair.first] = pair.second;
		}
	}
}

AxisOptions MouseAxes::GetAxisOptions(Direction_t axis)
{
	if (Options.empty())
	{
		return AxisOptions();
	}

	const auto it = Options.find(axis);

	if (it != Options.end())
	{
		return it->second;
	}

	// HACK: separate into its own cache
	AxisOptions result {};
	Options[axis] = result;
	return result;
}

bool MouseAxes::operator==(const MouseAxes& other) const
{
	return Directions == other.Directions
	       && Options == other.Options;
}
