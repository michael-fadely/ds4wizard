#pragma once

#include <array>
#include <cstdint>
#include <enum.h>

#define ENUM_FLAGS(TYPE) \
	std::string serializeFlags_ ## TYPE(TYPE ## _t value); \
	void deserializeFlags_ ## TYPE(const std::string& input, TYPE ## _t& value)

#define ENUM_SERIALIZE_FLAGS(TYPE) serializeFlags_ ## TYPE
#define ENUM_DESERIALIZE_FLAGS(TYPE) deserializeFlags_ ## TYPE

// TODO: make this a member of the struct
#define ENUM_VALUES(TYPE, LENGTH) extern const std::array<TYPE ## _t, LENGTH> TYPE ## _values

BETTER_ENUM(AxisPolarity, int,
            none,
            positive,
            negative)

/**
 * \brief Configuration for axis dead zone scaling.
 */
BETTER_ENUM(DeadZoneMode, int,
            /** \brief No dead zone is applied. */
            none,
            /** \brief When an axis reaches the dead zone threshold, the raw value is let through. */
            hardLimit,
            /**
             * \brief
             * When the dead zone threshold is reached, the axis output is scaled relative to that threshold (normalized).
             * Example: if the dead zone is 0.2 and an axis reaches or exceeds that value, it will be normalized to [0.0 .. 1.0].
             */
            scale)

// TODO: /!\ better name
BETTER_ENUM(SimulatorType, int, none, input, action)

// TODO: vibrate, set light color
BETTER_ENUM(ActionType, int, none, bluetoothDisconnect)

BETTER_ENUM(MouseButton, int,
            /** \brief The left mouse button. */
            left,
            /** \brief The right mouse button. */
            right,
            /** \brief The middle mouse button (the scroll click). */
            middle,
            /** \brief Extra mouse button 1, i.e. mouse button 4. */
            ex1,
            /** \brief Extra mouse button 2, i.e. mouse button 5. */
            ex2)

/**
 * \brief The type of input to poll from the device.
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

/**
 * \brief Bitfield representing Xbox 360 digital buttons.
 */
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

/**
 * \brief Bitfield representing Xbox 360 axes.
 */
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

/**
 * \brief Bitfield representing the direction of an action.
 */
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

/**
 * \brief The values that can be represented by a Hat Switch.
 */
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

#define DS4_BUTTONS_RAW  /* NOLINT(cppcoreguidelines-macro-usage) */ \
	square      = 1 << 4, \
	cross       = 1 << 5, \
	circle      = 1 << 6, \
	triangle    = 1 << 7, \
	l1          = 1 << 8,  \
	r1          = 1 << 9,  \
	l2          = 1 << 10, \
	r2          = 1 << 11, \
	share       = 1 << 12, \
	options     = 1 << 13, \
	l3          = 1 << 14, \
	r3          = 1 << 15, \
	ps          = 1 << 16, \
	touchButton = 1 << 17

using Ds4ButtonsRaw_t = uint32_t;

/**
 * \brief Bitfield representing the raw button status bits reported by the DualShock 4.
 */
struct Ds4ButtonsRaw
{
	enum T : Ds4ButtonsRaw_t
	{
		// first 4 bits are the hat switch
		hat = 0b1111,
		DS4_BUTTONS_RAW
	};

	static const Ds4ButtonsRaw_t mask = 0x0003FFFF;
	static const Ds4ButtonsRaw_t hat_mask = 0xF;

	/**
	 * \brief Get the Hat Switch portion of the bitfield and return it as a \c Hat enum.
	 * \param value Bitfield of buttons.
	 * \return The Hat Switch portion of the bitfield.
	 */
	static Hat getHat(Ds4ButtonsRaw_t value);

	static_assert(mask & touchButton, "nope");
	static_assert(mask & hat, "nope");
};

ENUM_VALUES(Ds4ButtonsRaw, 15);

using Ds4Buttons_t = uint32_t;

/**
 * \brief Serializable version of \sa Ds4ButtonsRaw to simplify
 * binding extra elements, such as touch sensor states and individual d-pad directions.
 */
struct Ds4Buttons
{
	enum T : Ds4Buttons_t
	{
		up    = 1 << 0,
		down  = 1 << 1,
		left  = 1 << 2,
		right = 1 << 3,
		
		DS4_BUTTONS_RAW,

		touch1 = 1 << 18,
		touch2 = 1 << 19
	};

	static const Ds4Buttons_t mask = 0x000FFFFF;
	static const Ds4Buttons_t dpad = up | down | left | right;

	static Ds4Buttons_t fromRaw(Ds4ButtonsRaw_t bits);

	static_assert(mask & right, "nope");
};

ENUM_FLAGS(Ds4Buttons);
ENUM_VALUES(Ds4Buttons, 20);

/**
 * \brief Bitfield representing the axes provided by the DualShock 4.
 */
struct Ds4Axes
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

using Ds4Axes_t = uint32_t;
ENUM_FLAGS(Ds4Axes);
ENUM_VALUES(Ds4Axes, 12);

/**
 * \brief Bitfield representing the (known) extensions reported by the DualShock 4.
 */
struct Ds4Extensions
{
	enum T : uint8_t
	{
		/** \brief A power/data cable is plugged into the controller. */
		cable = 1 << 0,
		/** \brief Heaphones/speakers are plugged into the controller's 3.5mm jack. */
		headphones = 1 << 1,
		/** \brief A microphone is plugged into the controller. */
		microphone = 1 << 2,
		/** \brief An unknown extension is connected to the controller. */
		unknown = 1 << 3
	};
};

using Ds4Extensions_t = uint8_t;
ENUM_FLAGS(Ds4Extensions);
ENUM_VALUES(Ds4Extensions, 4);
