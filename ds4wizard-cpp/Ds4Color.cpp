#include "stdafx.h"
#include "Ds4Color.h"
#include "lerp.h"

Ds4Color::Ds4Color(uint8_t r, uint8_t g, uint8_t b)
	: red(r),
	  green(g),
	  blue(b)
{
}

Ds4Color::Ds4Color(const Ds4Color& color)
	: red(color.red),
	  green(color.green),
	  blue(color.blue)
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

void Ds4Color::readJson(const QJsonObject& json)
{
	red   = json["r"].toInt();
	green = json["g"].toInt();
	blue  = json["b"].toInt();
}

void Ds4Color::writeJson(QJsonObject& json) const
{
	json["r"] = red;
	json["g"] = green;
	json["b"] = blue;
}
