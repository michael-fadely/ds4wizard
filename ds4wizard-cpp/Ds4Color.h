#pragma once

#include <cstdint>
#include "JsonData.h"

struct Ds4Color : JsonData
{
	uint8_t Red;
	uint8_t Green;
	uint8_t Blue;

	Ds4Color() = default;
	Ds4Color(uint8_t r, uint8_t g, uint8_t b);
	Ds4Color(const Ds4Color& color);

	bool operator==(const Ds4Color& other) const;
	bool operator!=(const Ds4Color& other) const;

	static Ds4Color lerp(const Ds4Color& a, const Ds4Color& b, float f);
	void readJson(const QJsonObject& json) override;
	void writeJson(QJsonObject& json) const override;
};
