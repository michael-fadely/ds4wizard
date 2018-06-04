#pragma once

#include "Ds4Color.h"
#include "JsonData.h"

class Ds4LightOptions : public JsonData
{
public:
	bool     AutomaticColor = true;
	Ds4Color Color;
	bool     IdleFade = true;
	Ds4Color LowBatteryColor;

	Ds4LightOptions(const Ds4Color& color);

	Ds4LightOptions(const Ds4LightOptions& other);

	Ds4LightOptions() = default;

	bool operator==(const Ds4LightOptions& other) const;
	bool operator!=(const Ds4LightOptions& other) const;
	void readJson(const QJsonObject& json) override;
	void writeJson(QJsonObject& json) const override;
};
