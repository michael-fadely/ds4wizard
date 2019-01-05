#include "pch.h"
#include "Ds4Color.h"
#include "lerp.h"

Ds4Color::Ds4Color(uint8_t r, uint8_t g, uint8_t b)
	: red(r),
	  green(g),
	  blue(b)
{
}

bool Ds4Color::operator==(const Ds4Color& other) const
{
	return red == other.red && green == other.green && blue == other.blue;
}

bool Ds4Color::operator!=(const Ds4Color& other) const
{
	return !(*this == other);
}

Ds4Color Ds4Color::lerp(const Ds4Color& a, const Ds4Color& b, float f)
{
	Ds4Color dest;

	dest.red   = ::lerp(a.red, b.red, f);
	dest.green = ::lerp(a.green, b.green, f);
	dest.blue  = ::lerp(a.blue, b.blue, f);

	return dest;
}

void Ds4Color::readJson(const nlohmann::json& json)
{
	red   = json["red"].get<int>();
	green = json["green"].get<int>();
	blue  = json["blue"].get<int>();
}

void Ds4Color::writeJson(nlohmann::json& json) const
{
	json["red"]   = red;
	json["green"] = green;
	json["blue"]  = blue;
}
