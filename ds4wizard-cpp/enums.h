#pragma once

#include <cstdint>
#include <enum.h>

#define ENUM_FLAGS(TYPE) \
	std::string serializeFlags_ ## TYPE(TYPE ## _t value); \
	void deserializeFlags_ ## TYPE(const std::string& input, TYPE ## _t& value)

#define ENUM_SERIALIZE_FLAGS(TYPE) serializeFlags_ ## TYPE
#define ENUM_DESERIALIZE_FLAGS(TYPE) deserializeFlags_ ## TYPE

#define ENUM_VALUES(TYPE, LENGTH) extern const TYPE ## _t TYPE ## _values[LENGTH]

BETTER_ENUM(AxisPolarity, int, none, positive, negative);

/**
 * \brief Configuration for axis dead zone scaling.
 * none:
 *     No dead zone is applied.
 * 
 * hardLimit:
 *     When an axis reaches the dead zone threshold, the raw value is let through.
 * 
 * scale:
 *     When the dead zone threshold is reached, the axis output is scaled relative to that threshold (normalized).
 *     Example: If the dead zone is 0.2 and an axis reaches or exceeds that value, it will be normalized to [0.0 .. 1.0].
 */
BETTER_ENUM(DeadZoneMode, int, none, hardLimit, scale);

// TODO: better name
BETTER_ENUM(SimulatorType, int, none, input, action);

BETTER_ENUM(ActionType, int, none, bluetoothDisconnect);

BETTER_ENUM(MouseButton, int, left, right, middle, ex1, ex2);

/**
 * \brief Bitfield representing an input source from real hardware (DualShock 4).
 */
struct InputType
{
	enum T : uint32_t
	{
		/** \brief None - invalid state. */
		none,
		/** \brief Requested input is a digital button. */
		button = 1 << 0,
		/** \brief Requested input is an axis. */
		axis = 1 << 1,
		/** \brief Requested input is a user-configured touch region. */
		touchRegion = 1 << 2
	};
};

/**
 * \brief Integral representation of \c InputType
 * \sa InputType
 */
using InputType_t = uint32_t;

ENUM_FLAGS(InputType);
ENUM_VALUES(InputType, 3);

/**
 * \brief Bitfield representing an output simulation type.
 */
struct OutputType // TODO: vjoy output support
{
	enum T : uint32_t
	{
		/** \brief None - invalid state. */
		none,
		/** \brief Desired output device is an XInput device. */
		xinput = 1 << 0,
		/** \brief Desired output device is a keyboard. */
		keyboard = 1 << 1,
		/** \brief Desired output device is a mouse. */
		mouse = 1 << 2
	};
};

using OutputType_t = uint32_t;

ENUM_FLAGS(OutputType);
ENUM_VALUES(OutputType, 3);

struct XInputButtons
{
	enum T : uint16_t
	{
		dPadUp        = 0x0001,
		dPadDown      = 0x0002,
		dPadLeft      = 0x0004,
		dPadRight     = 0x0008,
		start         = 0x0010,
		back          = 0x0020,
		leftThumb     = 0x0040,
		rightThumb    = 0x0080,
		leftShoulder  = 0x0100,
		rightShoulder = 0x0200,
		guide         = 0x0400,
		dummy         = 0x0800,
		a             = 0x1000,
		b             = 0x2000,
		x             = 0x4000,
		y             = 0x8000,
	};
};

using XInputButtons_t = uint16_t;
ENUM_FLAGS(XInputButtons);
ENUM_VALUES(XInputButtons, 16);

struct XInputAxis
{
	enum T : uint32_t
	{
		none,
		leftStickX   = 1 << 0,
		leftStickY   = 1 << 1,
		rightStickX  = 1 << 2,
		rightStickY  = 1 << 3,
		leftTrigger  = 1 << 4,
		rightTrigger = 1 << 5
	};
};

using XInputAxis_t = uint32_t;
ENUM_FLAGS(XInputAxis);
ENUM_VALUES(XInputAxis, 6);

struct Direction
{
	enum T : uint32_t
	{
		none,
		up    = 1 << 0,
		down  = 1 << 1,
		left  = 1 << 2,
		right = 1 << 3
	};
};

using Direction_t = uint32_t;
ENUM_FLAGS(Direction);
ENUM_VALUES(Direction, 4);

enum class Hat
{
	north,
	northEast,
	east,
	southEast,
	south,
	southWest,
	west,
	northWest,
	none
};

/**
 * \brief Indicates a digital button's pressed state.
 */
enum class PressedState
{
	/**
	 * \brief The button has been off for more than one poll.
	 */
	off,
	/**
	 * \brief The button was off last poll, but has now been pressed.
	 */
	pressed,
	/**
	 * \brief The button has been on (held) for more than one poll.
	 */
	on,
	/**
	 * \brief The button was on (held), but has now been released.
	 */
	released
};

struct Ds4Buttons
{
	enum T : uint32_t
	{
		square      = 1 << 0,
		cross       = 1 << 1,
		circle      = 1 << 2,
		triangle    = 1 << 3,
		l1          = 1 << 4,
		r1          = 1 << 5,
		l2          = 1 << 6,
		r2          = 1 << 7,
		share       = 1 << 8,
		options     = 1 << 9,
		l3          = 1 << 10,
		r3          = 1 << 11,
		ps          = 1 << 12,
		touchButton = 1 << 13,
		touch1      = 1 << 14,
		touch2      = 1 << 15,
		up          = 1 << 16,
		down        = 1 << 17,
		left        = 1 << 18,
		right       = 1 << 19
	};
};

using Ds4Buttons_t = uint32_t;

ENUM_FLAGS(Ds4Buttons);
ENUM_VALUES(Ds4Buttons, 20);

struct Ds4Axis
{
	enum T : uint32_t
	{
		leftStickX   = 1 << 0,
		leftStickY   = 1 << 1,
		rightStickX  = 1 << 2,
		rightStickY  = 1 << 3,
		leftTrigger  = 1 << 4,
		rightTrigger = 1 << 5,
		accelX       = 1 << 6,
		accelY       = 1 << 7,
		accelZ       = 1 << 8,
		gyroX        = 1 << 9,
		gyroY        = 1 << 10,
		gyroZ        = 1 << 11
	};
};

using Ds4Axis_t = uint32_t;
ENUM_FLAGS(Ds4Axis);
ENUM_VALUES(Ds4Axis, 12);

struct Ds4Extensions
{
	enum T : uint8_t
	{
		cable      = 1 << 0,
		headphones = 1 << 1,
		microphone = 1 << 2,
		unknown    = 1 << 3
	};
};

using Ds4Extensions_t = uint8_t;
ENUM_FLAGS(Ds4Extensions);
ENUM_VALUES(Ds4Extensions, 4);
