#pragma once

#include <cstdint>
#include <cmath>

#include "enums.h"

/// <summary>
/// A structure defining a DualShock 4 analog stick.
/// </summary>
struct Ds4Stick
{
	/// <summary>
	/// X axis of the DualShock 4 analog stick.
	/// Note that the center of the axis rests at 127, not 0.
	/// </summary>
	uint8_t X;

	/// <summary>
	/// Y axis of the DualShock 4 analog stick.
	/// Note that the center of the axis rests at 127, not 0.
	/// </summary>
	uint8_t Y;
	
	bool operator==(const Ds4Stick& other) const
	{
		return X == other.X && Y == other.Y;
	}

	bool operator!=(const Ds4Stick& right) const
	{
		return !(*this == right);
	}

	double LengthSquared()
	{
		return X * X + Y * Y;
	}

	double Length()
	{
		return std::sqrt(LengthSquared());
	}
};

/// <summary>
/// A structure defining a DualShock 4 vector with 2 signed short components.
/// Used for points on the touch pad.
/// </summary>
struct Ds4Vector2
{
	/// <summary>
	/// The X component of the point.
	/// Left is 0, center is 959, right is 1919.
	/// </summary>
	short X;

	/// <summary>
	/// The Y component of the point.
	/// Top is 0, center is 470, bottom is 942.
	/// </summary>
	short Y;

	bool operator==(Ds4Vector2 other) const
	{
		return X == other.X && Y == other.Y;
	}

	double LengthSquared() const
	{
		return X * X + Y * Y;
	}

	double Length() const
	{
		return std::sqrt(LengthSquared());
	}
};

/// <summary>
/// A structure defining a DualShock 4 vector with 3 signed short components.
/// Used for the gyroscope and accelerometer.
/// </summary>
struct Ds4Vector3
{
	short X, Y, Z;

	bool operator==(Ds4Vector3 other) const
	{
		return X == other.X && Y == other.Y && Z == other.Z;
	}

	double LengthSquared() const
	{
		return X * X + Y * Y + Z * Z;
	}

	double Length() const
	{
		return std::sqrt(LengthSquared());
	}
};

struct Ds4InputData
{
	Ds4Stick   LeftStick;
	Ds4Stick   RightStick;
	Hat        DPad;
	uint8_t    Square;
	uint8_t    Cross;
	uint8_t    Circle;
	uint8_t    Triangle;
	uint8_t    L1;
	uint8_t    R1;
	uint8_t    L2;
	uint8_t    R2;
	uint8_t    Share;
	uint8_t    Options;
	uint8_t    L3;
	uint8_t    R3;
	uint8_t    PS;
	uint8_t    TouchButton;
	uint8_t    FrameCount;
	uint8_t    LeftTrigger;
	uint8_t    RightTrigger;
	uint8_t    Battery;
	bool       Charging;
	Ds4Vector3 Accel;
	Ds4Vector3 Gyro;
	uint8_t    Extensions;
	uint8_t    TouchEvent;
	uint8_t    TouchFrame;
	bool       Touch1;
	uint8_t    Touch1Id;
	Ds4Vector2 TouchPoint1;
	bool       Touch2;
	uint8_t    Touch2Id;
	Ds4Vector2 TouchPoint2;
	Ds4Vector2 LastTouchPoint1;
	Ds4Vector2 LastTouchPoint2;
};
