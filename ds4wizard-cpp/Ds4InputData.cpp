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

bool Ds4Vector2::operator!=(const Ds4Vector2& other) const
{
	return !(*this == other);
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

bool Ds4Vector3::operator!=(const Ds4Vector3& other) const
{
	return !(*this == other);
}

float Ds4Vector3::lengthSquared() const
{
	return (float)x * (float)x + (float)y * (float)y + (float)z * (float)z;
}

float Ds4Vector3::length() const
{
	return std::sqrt(lengthSquared());
}

bool Ds4InputData::operator==(const Ds4InputData& other) const
{
	return frameCount      == other.frameCount &&
	       leftStick       == other.leftStick &&
	       rightStick      == other.rightStick &&
	       dPad            == other.dPad &&
	       square          == other.square &&
	       cross           == other.cross &&
	       circle          == other.circle &&
	       triangle        == other.triangle &&
	       l1              == other.l1 &&
	       r1              == other.r1 &&
	       l2              == other.l2 &&
	       r2              == other.r2 &&
	       share           == other.share &&
	       options         == other.options &&
	       l3              == other.l3 &&
	       r3              == other.r3 &&
	       ps              == other.ps &&
	       touchButton     == other.touchButton &&
	       leftTrigger     == other.leftTrigger &&
	       rightTrigger    == other.rightTrigger &&
	       battery         == other.battery &&
	       charging        == other.charging &&
	       accel           == other.accel &&
	       gyro            == other.gyro &&
	       extensions      == other.extensions &&
	       touchEvent      == other.touchEvent &&
	       touchFrame      == other.touchFrame &&
	       touch1          == other.touch1 &&
	       touch1Id        == other.touch1Id &&
	       touchPoint1     == other.touchPoint1 &&
	       touch2          == other.touch2 &&
	       touch2Id        == other.touch2Id &&
	       touchPoint2     == other.touchPoint2 &&
	       lastTouchPoint1 == other.lastTouchPoint1 &&
	       lastTouchPoint2 == other.lastTouchPoint2;
}

bool Ds4InputData::operator!=(const Ds4InputData& other) const
{
	return !(*this == other);
}
