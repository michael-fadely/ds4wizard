#pragma once

#include "Ds4Color.h"
#include "JsonData.h"

class Ds4LightOptions : public JsonData
{
public:
	bool automaticColor = true;
	Ds4Color color;
	// TODO: move to idle options?
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
