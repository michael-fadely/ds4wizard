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

float Ds4Stick::lengthSquared() const
{
	return (float)x * (float)x + (float)y * (float)y;
}

float Ds4Stick::length() const
{
	return std::sqrt(lengthSquared());
}

bool Ds4Vector2::operator==(const Ds4Vector2& other) const
{
	return x == other.x && y == other.y;
}

float Ds4Vector2::lengthSquared() const
{
	return (float)x * (float)x + (float)y * (float)y;
}

float Ds4Vector2::length() const
{
	return std::sqrt(lengthSquared());
}

bool Ds4Vector3::operator==(const Ds4Vector3& other) const
{
	return x == other.x && y == other.y && z == other.z;
}

float Ds4Vector3::lengthSquared() const
{
	return (float)x * (float)x + (float)y * (float)y + (float)z * (float)z;
}

float Ds4Vector3::length() const
{
	return std::sqrt(lengthSquared());
}
