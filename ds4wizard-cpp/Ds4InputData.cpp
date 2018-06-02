#include "stdafx.h"
#include "Ds4InputData.h"
#include <cmath>

bool Ds4Stick::operator==(const Ds4Stick& other) const
{
	return X == other.X && Y == other.Y;
}

bool Ds4Stick::operator!=(const Ds4Stick& right) const
{
	return !(*this == right);
}

double Ds4Stick::LengthSquared() const
{
	return X * X + Y * Y;
}

double Ds4Stick::Length() const
{
	return std::sqrt(LengthSquared());
}

bool Ds4Vector2::operator==(Ds4Vector2 other) const
{
	return X == other.X && Y == other.Y;
}

double Ds4Vector2::LengthSquared() const
{
	return X * X + Y * Y;
}

double Ds4Vector2::Length() const
{
	return std::sqrt(LengthSquared());
}

bool Ds4Vector3::operator==(Ds4Vector3 other) const
{
	return X == other.X && Y == other.Y && Z == other.Z;
}

double Ds4Vector3::LengthSquared() const
{
	return X * X + Y * Y + Z * Z;
}

double Ds4Vector3::Length() const
{
	return std::sqrt(LengthSquared());
}
