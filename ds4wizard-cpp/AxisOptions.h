#pragma once

#include "enums.h"
#include "JsonData.h"
#include <optional>

class Vector3;
class Vector2;

/**
 * \brief Common configuration for an axis.
 */
class AxisOptions : public JsonData
{
public:
	/**
	 * \brief Multiplier for the axis, or \c std::nullopt
	 */
	std::optional<float> multiplier;

	/**
	 * \brief Polarity of the axis, or \c std::nullopt
	 * \sa AxisPolarity
	 */
	std::optional<AxisPolarity> polarity;

	AxisOptions() = default;
	AxisOptions(const AxisOptions&) = default;
	AxisOptions& operator=(const AxisOptions&) = default;

	/**
	 * \brief Constructs new axis configuration with the specified polarity.
	 * \param polarity Polarity of the simulated axis.
	 */
	explicit AxisOptions(AxisPolarity polarity);

	bool operator==(const AxisOptions& other) const;
	bool operator!=(const AxisOptions& other) const;
	void readJson(const nlohmann::json& json) override;
	void writeJson(nlohmann::json& json) const override;
};

/**
 * \brief Configuration for an axis read from real hardware.
 */
class InputAxisOptions : public AxisOptions
{
public:
	/**
	 * \brief Optionally invert the axis output.
	 */
	std::optional<bool> invert;

	/**
	 * \brief The dead zone mode to use if configured.
	 * If \c DeadZoneSource::none or \c std::nullopt, no
	 * dead zone will be applied to the axis.
	 * \sa DeadZoneSource, deadZone
	 */
	std::optional<DeadZoneSource> deadZoneSource;

	/**
	 * \brief Dead zone threshold.
	 * \sa deadZoneSource
	 */
	std::optional<float> deadZone;

	InputAxisOptions() = default;
	InputAxisOptions(const InputAxisOptions&) = default;
	InputAxisOptions& operator=(const InputAxisOptions&) = default;

	/**
	 * \brief Constructs an axis configuration with the given polarity.
	 * \param polarity The polarity of the axis to accept.
	 */
	explicit InputAxisOptions(AxisPolarity polarity);

private:
	/**
	 * \brief Check if \a value exceeds configured dead zone, if any.
	 */
	[[nodiscard]] bool exceedsDeadZone(float value) const;

public:
	// TODO: better documentation - i.e. axisValue is returned as-is sometimes
	/**
	 * \brief Returns the provided analog input with the stored multiplier, inverse, and dead zone applied.
	 * 
	 * If \c deadZoneSource is \c DeadZoneSource::axisVectorMagnitude, \a axisVectorMagnitude is used for activation,
	 * and \a axisValue is returned only with options applied (i.e. without dead zone scaling applied).
	 *
	 * Otherwise, \a axisVector is returned with options and dead zone scaling applied.
	 */
	[[nodiscard]] float applyToValueWithMagnitude(float axisValue, float axisVectorMagnitude) const;

	/**
	 * \brief Returns the value with multiplier, inverse, and dead zone applied.
	 */
	[[nodiscard]] float applyToValue(float value) const;

	/**
	 * \brief Returns the provided analog input with the stored multiplier, inverse, and dead zone applied.
	 * 
	 * If \c deadZoneSource is \c DeadZoneSource::axisVectorMagnitude, the magnitude
	 * of \a axisVector is used for activation, and \a axisValue is returned only with options
	 * applied (i.e. without dead zone scaling applied).
	 *
	 * Otherwise, \a axisVector is returned with options and dead zone scaling applied.
	 * 
	 * \param axisValue The raw axis value provided by the device, clamped according to polarity.
	 * \param axisVector The vector that the axis value belongs to.
	 * \return The analog value with options applied.
	 */
	[[nodiscard]] float applyToValue(float axisValue, const Vector2& axisVector) const;

	/**
	 * \brief Returns the provided analog input with the stored multiplier, inverse, and dead zone applied.
	 *
	 * If \c deadZoneSource is \c DeadZoneSource::axisVectorMagnitude, the magnitude
	 * of \a axisVector is used for activation, and \a axisValue is returned only with options
	 * applied (i.e. without dead zone scaling applied).
	 *
	 * Otherwise, \a axisVector is returned with options and dead zone scaling applied.
	 *
	 * \param axisValue The raw axis value provided by the device, clamped according to polarity.
	 * \param axisVector The vector that the axis value belongs to.
	 * \return The analog value with options applied.
	 */
	[[nodiscard]] float applyToValue(float axisValue, const Vector3& axisVector) const;

	bool operator==(const InputAxisOptions& other) const;
	bool operator!=(const InputAxisOptions& other) const;

	void readJson(const nlohmann::json& json) override;
	void writeJson(nlohmann::json& json) const override;
};

/**
 * \brief Describes configuration for one or more XInput axes.
 */
class XInputAxes : public JsonData
{
public:
	/**
	 * \brief Bitfield representing all the affected XInput axes.
	 * \sa XInputAxis_t
	 */
	XInputAxis_t axes = 0;

	/**
	 * \brief Configurations for each XInput axis.
	 */
	std::unordered_map<XInputAxis_t, AxisOptions> options;

	XInputAxes() = default;
	XInputAxes(const XInputAxes&) = default;
	XInputAxes& operator=(const XInputAxes&) = default;

	/**
	 * \brief Retrieves configuration for the provided XInput axis.
	 * \param axis The axis to retrieve the configuration for.
	 * \return The configuration for the requested axis, or empty configuration if not found.
	 */
	[[nodiscard]] AxisOptions getAxisOptions(XInputAxis::T axis) const;

	bool operator==(const XInputAxes& other) const;
	bool operator!=(const XInputAxes& other) const;
	void readJson(const nlohmann::json& json) override;
	void writeJson(nlohmann::json& json) const override;
};

/**
 * \brief Describes configuration for one or more mouse movement directions and treats them as axes.
 */
class MouseAxes : public JsonData
{
public:
	/**
	 * \brief Bitfield representing all the affected mouse direction.
	 */
	Direction_t directions = 0;

	/**
	 * \brief Configuration for each mouse direction.
	 */
	std::unordered_map<Direction_t, AxisOptions> options;

	MouseAxes() = default;
	MouseAxes(const MouseAxes&) = default;
	MouseAxes& operator=(const MouseAxes&) = default;

	/**
	 * \brief Retrieves configuration for the provided mouse axis.
	 * \param axis The axis to retrieve the configuration for.
	 * \return The configuration for the requested axis, or empty configuration if not found.
	 */
	AxisOptions getAxisOptions(Direction_t axis);

	bool operator==(const MouseAxes& other) const;
	void readJson(const nlohmann::json& json) override;
	void writeJson(nlohmann::json& json) const override;
};
