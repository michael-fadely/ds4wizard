#pragma once

#include <cstdint>
#include <enum.h>

#define serializableFlags(TYPE) \
	std::string serializeFlags_ ## TYPE ## (TYPE ## _t value); \
	void deserializeFlags_ ## TYPE ## (const std::string& input, TYPE ## _t& value)

#define serializeFlags(TYPE) serializeFlags_ ## TYPE
#define deserializeFlags(TYPE) deserializeFlags_ ## TYPE

BETTER_ENUM(AxisPolarity, int, none, positive, negative);
BETTER_ENUM(DeadZoneMode, int, none, hardLimit, scale);

// TODO: better name
BETTER_ENUM(SimulatorType, int, none, input, action);

BETTER_ENUM(ActionType, int, none, bluetoothDisconnect);

struct InputType
{
	enum T
	{
		None,
		button      = 1 << 0,
		axis        = 1 << 1,
		touchRegion = 1 << 2
	};
};

using InputType_t = uint32_t;

serializableFlags(InputType);

struct OutputType // TODO: vjoy output support
{
	enum T
	{
		None,
		XInput   = 1 << 0,
		Keyboard = 1 << 1,
		Mouse    = 1 << 2
	};
};

using OutputType_t = uint32_t;

serializableFlags(OutputType);

struct XInputButtons
{
	enum T : uint16_t
	{
		DPadUp        = 0x0001,
		DPadDown      = 0x0002,
		DPadLeft      = 0x0004,
		DPadRight     = 0x0008,
		Start         = 0x0010,
		Back          = 0x0020,
		LeftThumb     = 0x0040,
		RightThumb    = 0x0080,
		LeftShoulder  = 0x0100,
		RightShoulder = 0x0200,
		Guide         = 0x0400,
		Dummy         = 0x0800,
		A             = 0x1000,
		B             = 0x2000,
		X             = 0x4000,
		Y             = 0x8000,
	};
};

using XInputButtons_t = uint16_t;
serializableFlags(XInputButtons);

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
serializableFlags(XInputAxis);

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
serializableFlags(Direction);

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

/// <summary>
/// Indicates a digital button's pressed state.
/// </summary>
enum class PressedState
{
	/// <summary>
	/// The button has been off for more than one poll.
	/// </summary>
	off,
	/// <summary>
	/// The button was off last poll, but has now been pressed.
	/// </summary>
	pressed,
	/// <summary>
	/// The button has been on (held) for more than one poll.
	/// </summary>
	on,
	/// <summary>
	/// The button was on (held), but has now been released.
	/// </summary>
	released
};

struct Ds4Buttons
{
	enum T : uint32_t
	{
		Square      = 1 << 0,
		Cross       = 1 << 1,
		Circle      = 1 << 2,
		Triangle    = 1 << 3,
		L1          = 1 << 4,
		R1          = 1 << 5,
		L2          = 1 << 6,
		R2          = 1 << 7,
		Share       = 1 << 8,
		Options     = 1 << 9,
		L3          = 1 << 10,
		R3          = 1 << 11,
		PS          = 1 << 12,
		TouchButton = 1 << 13,
		Touch1      = 1 << 14,
		Touch2      = 1 << 15,
		Up          = 1 << 16,
		Down        = 1 << 17,
		Left        = 1 << 18,
		Right       = 1 << 19
	};
};

using Ds4Buttons_t = uint32_t;

serializableFlags(Ds4Buttons);

struct Ds4Axis
{
	enum T : uint32_t
	{
		LeftStickX   = 1 << 0,
		LeftStickY   = 1 << 1,
		RightStickX  = 1 << 2,
		RightStickY  = 1 << 3,
		LeftTrigger  = 1 << 4,
		RightTrigger = 1 << 5,
		AccelX       = 1 << 6,
		AccelY       = 1 << 7,
		AccelZ       = 1 << 8,
		GyroX        = 1 << 9,
		GyroY        = 1 << 10,
		GyroZ        = 1 << 11
	};
};

using Ds4Axis_t = uint32_t;
serializableFlags(Ds4Axis);

struct Ds4Extensions
{
	enum T : uint8_t
	{
		Cable      = 1 << 0,
		Headphones = 1 << 1,
		Microphone = 1 << 2,
		Unknown    = 1 << 3
	};
};

using Ds4Extensions_t = uint8_t;
serializableFlags(Ds4Extensions);
