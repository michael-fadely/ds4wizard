#pragma once

#include "Ds4Color.h"
#include "JsonData.h"

/**
 * \brief Configuration for the \c Ds4Device light bar.
 * \sa Ds4Device, Ds4Color
 */
class Ds4LightOptions : public JsonData
{
public:
	/**
	 * \brief If \c true, a color is automatically assigned based on device connection order.
	 * \sa Ds4AutoLightColor
	 */
	bool automaticColor = true;

	/**
	 * \brief The user-defined light bar color.
	 */
	Ds4Color color;

	// TODO: move to idle options?
	/**
	 * \brief If \c true, the light fades out proportionally to its idle time (elapsed / threshold).
	 */
	bool idleFade = true;

	explicit Ds4LightOptions(const Ds4Color& color);
	Ds4LightOptions(const Ds4LightOptions& other);
	Ds4LightOptions() = default;

	bool operator==(const Ds4LightOptions& other) const;
	bool operator!=(const Ds4LightOptions& other) const;
	Ds4LightOptions& operator=(const Ds4LightOptions& other) = default;

	void readJson(const nlohmann::json& json) override;
	void writeJson(nlohmann::json& json) const override;
};
