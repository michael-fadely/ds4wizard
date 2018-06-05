#pragma once

#include "enums.h"
#include "JsonData.h"

class AxisOptions : public JsonData
{
public:
	float Multiplier      = 1.0f;
	AxisPolarity Polarity = AxisPolarity::positive;

	AxisOptions() = default;

	explicit AxisOptions(AxisPolarity polarity);
	AxisOptions(const AxisOptions& other);

	bool operator==(const AxisOptions& other) const;
	bool operator!=(const AxisOptions& other) const;
	virtual void readJson(const QJsonObject& json) override;
	virtual void writeJson(QJsonObject& json) const override;
};

class InputAxisOptions : public AxisOptions
{
public:
	bool         invert       = false;
	DeadZoneMode deadZoneMode = DeadZoneMode::scale;
	float        deadZone     = 0.0f;

	InputAxisOptions() = default;
	explicit InputAxisOptions(AxisPolarity polarity);
	InputAxisOptions(const InputAxisOptions& other);

	void ApplyDeadZone(float& analog) const;

	bool operator==(const InputAxisOptions& other) const;
	bool operator!=(const InputAxisOptions& other) const;

	void readJson(const QJsonObject& json) override;
	void writeJson(QJsonObject& json) const override;
};

class XInputAxes : public JsonData
{
public:
	XInputAxis_t Axes = 0;
	std::unordered_map<XInputAxis_t, AxisOptions> Options;

	XInputAxes() = default;
	XInputAxes(const XInputAxes& other);

	AxisOptions GetAxisOptions(XInputAxis::T axis);

	bool operator==(const XInputAxes& other) const;
	bool operator!=(const XInputAxes& other) const;
	void readJson(const QJsonObject& json) override;
	void writeJson(QJsonObject& json) const override;
};

class MouseAxes : public JsonData
{
public:
	Direction_t Directions = 0;
	std::unordered_map<Direction_t, AxisOptions> Options;

	MouseAxes() = default;
	MouseAxes(const MouseAxes& other);

	AxisOptions GetAxisOptions(Direction_t axis);

	bool operator==(const MouseAxes& other) const;
	void readJson(const QJsonObject& json) override;
	void writeJson(QJsonObject& json) const override;
};
