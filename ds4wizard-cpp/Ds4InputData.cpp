#include "stdafx.h"
#include "Ds4InputData.h"
#include <cmath>

bool Ds4Stick::operator==(const Ds4Stick& other) const
{
	return x == other.x && y == other.y;
}

bool Ds4Stick::operator!=(const Ds4Stick& right) const
{
	return !(*this == right);
}

double Ds4Stick::lengthSquared() const
{
	return x * x + y * y;
}

double Ds4Stick::length() const
{
	return std::sqrt(lengthSquared());
}

bool Ds4Vector2::operator==(const Ds4Vector2& other) const
{
	return x == other.x && y == other.y;
}

double Ds4Vector2::lengthSquared() const
{
	return x * x + y * y;
}

double Ds4Vector2::length() const
{
	return std::sqrt(lengthSquared());
}

bool Ds4Vector3::operator==(const Ds4Vector3& other) const
{
	return x == other.x && y == other.y && z == other.z;
}

double Ds4Vector3::lengthSquared() const
{
	return x * x + y * y + z * z;
}

double Ds4Vector3::length() const
{
	return std::sqrt(lengthSquared());
}
