#pragma once

#include <cstdint>

struct InputType
{
	enum T
	{
		None,
		Button      = 1 << 0,
		Axis        = 1 << 1,
		TouchRegion = 1 << 2
	};
};

using InputType_t = uint32_t;

enum class AxisPolarity
{
	None,
	Positive,
	Negative
};

// TODO: better name
enum class SimulatorType
{
	None,
	Input,
	Action
};

enum class ActionType
{
	None,
	BluetoothDisconnect
};

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

enum class DeadZoneMode
{
	None,
	HardLimit,
	Scale
};

struct XInputAxis
{
	enum T : uint32_t
	{
		None,
		LeftStickX   = 1 << 0,
		LeftStickY   = 1 << 1,
		RightStickX  = 1 << 2,
		RightStickY  = 1 << 3,
		LeftTrigger  = 1 << 4,
		RightTrigger = 1 << 5
	};
};

using XInputAxis_t = uint32_t;

struct Direction
{
	enum T : uint32_t
	{
		None,
		Up    = 1 << 0,
		Down  = 1 << 1,
		Left  = 1 << 2,
		Right = 1 << 3
	};
};

using Direction_t = uint32_t;

enum class Hat
{
	North,
	NorthEast,
	East,
	SouthEast,
	South,
	SouthWest,
	West,
	NorthWest,
	None
};

/// <summary>
/// Indicates a digital button's pressed state.
/// </summary>
enum class PressedState
{
	/// <summary>
	/// The button has been off for more than one poll.
	/// </summary>
	Off,
	/// <summary>
	/// The button was off last poll, but has now been pressed.
	/// </summary>
	Pressed,
	/// <summary>
	/// The button has been on (held) for more than one poll.
	/// </summary>
	On,
	/// <summary>
	/// The button was on (held), but has now been released.
	/// </summary>
	Released
};

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
