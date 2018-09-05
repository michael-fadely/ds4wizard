#pragma once

#include "enums.h"
#include "JsonData.h"
#include <optional>

class AxisOptions : public JsonData
{
public:
	std::optional<float> multiplier;
	std::optional<AxisPolarity> polarity;

	AxisOptions() = default;
	AxisOptions(const AxisOptions&) = default;
	AxisOptions& operator=(const AxisOptions&) = default;

	explicit AxisOptions(AxisPolarity polarity);

	bool operator==(const AxisOptions& other) const;
	bool operator!=(const AxisOptions& other) const;
	virtual void readJson(const nlohmann::json& json) override;
	virtual void writeJson(nlohmann::json& json) const override;
};

class InputAxisOptions : public AxisOptions
{
public:
	std::optional<bool> invert;
	std::optional<DeadZoneMode> deadZoneMode;
	std::optional<float> deadZone;

	InputAxisOptions() = default;
	InputAxisOptions(const InputAxisOptions&) = default;
	InputAxisOptions& operator=(const InputAxisOptions&) = default;

	explicit InputAxisOptions(AxisPolarity polarity);

	void applyDeadZone(float& analog) const;

	bool operator==(const InputAxisOptions& other) const;
	bool operator!=(const InputAxisOptions& other) const;

	void readJson(const nlohmann::json& json) override;
	void writeJson(nlohmann::json& json) const override;
};

class XInputAxes : public JsonData
{
public:
	XInputAxis_t axes = 0;
	std::unordered_map<XInputAxis_t, AxisOptions> options;

	XInputAxes() = default;
	XInputAxes(const XInputAxes&) = default;
	XInputAxes& operator=(const XInputAxes&) = default;

	AxisOptions getAxisOptions(XInputAxis::T axis);

	bool operator==(const XInputAxes& other) const;
	bool operator!=(const XInputAxes& other) const;
	void readJson(const nlohmann::json& json) override;
	void writeJson(nlohmann::json& json) const override;
};

class MouseAxes : public JsonData
{
public:
	Direction_t directions = 0;
	std::unordered_map<Direction_t, AxisOptions> options;

	MouseAxes() = default;
	MouseAxes(const MouseAxes&) = default;
	MouseAxes& operator=(const MouseAxes&) = default;

	AxisOptions getAxisOptions(Direction_t axis);

	bool operator==(const MouseAxes& other) const;
	void readJson(const nlohmann::json& json) override;
	void writeJson(nlohmann::json& json) const override;
};
