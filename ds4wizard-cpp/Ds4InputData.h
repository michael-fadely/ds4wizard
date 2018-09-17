#pragma once

#include <cstdint>

#include "enums.h"

/**
 * \brief 
 * A structure defining a DualShock 4 analog stick.
 */
struct Ds4Stick
{
	/**
	 * \brief 
	 * X axis of the DualShock 4 analog stick.
	 * Note that the center of the axis rests at 127, not 0.
	 */
	uint8_t x;

	/**
	 * \brief 
	 * Y axis of the DualShock 4 analog stick.
	 * Note that the center of the axis rests at 127, not 0.
	 */
	uint8_t y;

	bool operator==(const Ds4Stick& other) const;
	bool operator!=(const Ds4Stick& right) const;

	float lengthSquared() const;
	float length() const;
};

/**
 * \brief 
 * A structure defining a DualShock 4 vector with 2 signed short components.
 * Used for points on the touch pad.
 */
struct Ds4Vector2
{
	/**
	 * \brief 
	 * The X component of the point.
	 * Left is \c 0, center is \c 959, right is \c 1919.
	 */
	short x;

	/**
	 * \brief 
	 * The Y component of the point.
	 * Top is \c 0, center is \c 470, bottom is \c 942.
	 */
	short y;

	bool operator==(const Ds4Vector2& other) const;
	bool operator!=(const Ds4Vector2& other) const;

	float lengthSquared() const;
	float length() const;
};

/**
 * \brief 
 * A structure defining a DualShock 4 vector with 3 signed \c short components.
 * Used for the gyroscope and accelerometer.
 */
struct Ds4Vector3
{
	short x, y, z;

	bool operator==(const Ds4Vector3& other) const;
	bool operator!=(const Ds4Vector3& other) const;

	float lengthSquared() const;
	float length() const;
};

struct Ds4InputData
{
	Ds4Stick   leftStick;
	Ds4Stick   rightStick;
	Hat        dPad;
	bool       square;
	bool       cross;
	bool       circle;
	bool       triangle;
	bool       l1;
	bool       r1;
	bool       l2;
	bool       r2;
	bool       share;
	bool       options;
	bool       l3;
	bool       r3;
	bool       ps;
	bool       touchButton;
	uint8_t    frameCount;
	uint8_t    leftTrigger;
	uint8_t    rightTrigger;
	uint8_t    battery;
	Ds4Vector3 accel;
	Ds4Vector3 gyro;
	uint8_t    extensions;
	uint8_t    touchEvent;
	uint8_t    touchFrame;
	bool       touch1;
	uint8_t    touch1Id;
	Ds4Vector2 touchPoint1;
	bool       touch2;
	uint8_t    touch2Id;
	Ds4Vector2 touchPoint2;
	Ds4Vector2 lastTouchPoint1;
	Ds4Vector2 lastTouchPoint2;

	bool operator==(const Ds4InputData& other) const;
	bool operator!=(const Ds4InputData& other) const;
};
