#include "stdafx.h"
#include "Ds4Color.h"
#include "lerp.h"

Ds4Color::Ds4Color(uint8_t r, uint8_t g, uint8_t b)
{
	Red   = r;
	Green = g;
	Blue  = b;
}

Ds4Color::Ds4Color(const Ds4Color& color)
{
	Red   = color.Red;
	Green = color.Green;
	Blue  = color.Blue;
}

bool Ds4Color::operator==(const Ds4Color& other) const
{
	return Red == other.Red && Green == other.Green && Blue == other.Blue;
}

bool Ds4Color::operator!=(const Ds4Color& other) const
{
	return !(*this == other);
}

Ds4Color Ds4Color::lerp(const Ds4Color& a, const Ds4Color& b, float f)
{
	Ds4Color dest;

	dest.Red   = ::lerp(a.Red, b.Red, f);
	dest.Green = ::lerp(a.Green, b.Green, f);
	dest.Blue  = ::lerp(a.Blue, b.Blue, f);

	return dest;
}
