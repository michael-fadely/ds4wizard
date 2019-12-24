#include "pch.h"
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

int Ds4Stick::lengthSquared() const
{
	const auto xi = static_cast<int>(x);
	const auto yi = static_cast<int>(y);
	return xi * xi + yi * yi;
}

float Ds4Stick::length() const
{
	return std::sqrt(static_cast<float>(lengthSquared()));
}

bool Ds4Vector2::operator==(const Ds4Vector2& other) const
{
	return x == other.x && y == other.y;
}

bool Ds4Vector2::operator!=(const Ds4Vector2& other) const
{
	return !(*this == other);
}

int Ds4Vector2::lengthSquared() const
{
	const auto xi = static_cast<int>(x);
	const auto yi = static_cast<int>(y);
	return xi * xi + yi * yi;
}

float Ds4Vector2::length() const
{
	return std::sqrt(static_cast<float>(lengthSquared()));
}

bool Ds4Vector3::operator==(const Ds4Vector3& other) const
{
	return x == other.x && y == other.y && z == other.z;
}

bool Ds4Vector3::operator!=(const Ds4Vector3& other) const
{
	return !(*this == other);
}

int Ds4Vector3::lengthSquared() const
{
	const auto xi = static_cast<int>(x);
	const auto yi = static_cast<int>(y);
	const auto zi = static_cast<int>(z);
	
	return xi * xi + yi * yi + zi * zi;
}

float Ds4Vector3::length() const
{
	return static_cast<float>(std::sqrt(static_cast<double>(lengthSquared())));
}

Hat Ds4InputData::dPad() const
{
	return static_cast<Hat>(activeButtons & Ds4Buttons::dpad);
}

bool Ds4InputData::square() const
{
	return !!(activeButtons & Ds4Buttons::square);
}

bool Ds4InputData::cross() const
{
	return !!(activeButtons & Ds4Buttons::cross);
}

bool Ds4InputData::circle() const
{
	return !!(activeButtons & Ds4Buttons::circle);
}

bool Ds4InputData::triangle() const
{
	return !!(activeButtons & Ds4Buttons::triangle);
}

bool Ds4InputData::l1() const
{
	return !!(activeButtons & Ds4Buttons::l1);
}

bool Ds4InputData::r1() const
{
	return !!(activeButtons & Ds4Buttons::r1);
}

bool Ds4InputData::l2() const
{
	return !!(activeButtons & Ds4Buttons::l2);
}

bool Ds4InputData::r2() const
{
	return !!(activeButtons & Ds4Buttons::r2);
}

bool Ds4InputData::share() const
{
	return !!(activeButtons & Ds4Buttons::share);
}

bool Ds4InputData::options() const
{
	return !!(activeButtons & Ds4Buttons::options);
}

bool Ds4InputData::l3() const
{
	return !!(activeButtons & Ds4Buttons::l3);
}

bool Ds4InputData::r3() const
{
	return !!(activeButtons & Ds4Buttons::r3);
}

bool Ds4InputData::ps() const
{
	return !!(activeButtons & Ds4Buttons::ps);
}

bool Ds4InputData::touchButton() const
{
	return !!(activeButtons & Ds4Buttons::touchButton);
}

bool Ds4InputData::operator==(const Ds4InputData& other) const
{
	return frameCount      == other.frameCount &&
	       activeButtons   == other.activeButtons &&
	       leftStick       == other.leftStick &&
	       rightStick      == other.rightStick &&
	       leftTrigger     == other.leftTrigger &&
	       rightTrigger    == other.rightTrigger &&
	       battery         == other.battery &&
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
