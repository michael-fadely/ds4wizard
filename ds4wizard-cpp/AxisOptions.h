#pragma once

#include "enums.h"

class AxisOptions
{
public:
	float Multiplier      = 1.0f;
	AxisPolarity Polarity = AxisPolarity::Positive;

	AxisOptions() = default;

	explicit AxisOptions(AxisPolarity polarity);
	AxisOptions(const AxisOptions& other);

	bool operator==(const AxisOptions& other) const;
	bool operator!=(const AxisOptions& other) const;
};

class InputAxisOptions : public AxisOptions
{
public:
	bool         Invert       = false;
	DeadZoneMode DeadZoneMode = DeadZoneMode::Scale;
	float        DeadZone     = 0.0f;

	InputAxisOptions() = default;
	explicit InputAxisOptions(AxisPolarity polarity);
	InputAxisOptions(const InputAxisOptions& other);

	void ApplyDeadZone(float& analog) const;

	bool operator==(const InputAxisOptions& other) const;
	bool operator!=(const InputAxisOptions& other) const;

};

class XInputAxes
{
public:
	XInputAxis_t Axes;
	std::unordered_map<XInputAxis_t, AxisOptions> Options;

	XInputAxes() = default;
	XInputAxes(const XInputAxes& other);

	AxisOptions GetAxisOptions(XInputAxis::T axis);

	bool operator==(const XInputAxes& other) const;
	bool operator!=(const XInputAxes& other) const;
};

class MouseAxes
{
public:
	Direction_t Directions = 0;
	std::unordered_map<Direction_t, AxisOptions> Options;

	MouseAxes() = default;
	MouseAxes(const MouseAxes& other);

	AxisOptions GetAxisOptions(Direction_t axis);

	bool operator==(const MouseAxes& other) const;
};
