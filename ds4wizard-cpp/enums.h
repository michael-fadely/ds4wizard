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
 * \brief Configuration for the source of the value used in dead zone calculations.
 */
BETTER_ENUM(DeadZoneSource, int,
            /** \brief No dead zone is applied. */
            none,

            /** \brief Use the magnitude of the vector that the input belongs to.
             *
             * e.g. If the left stick is the input, the magnitude of the left stick's
             * X and Y values must exceed the dead zone value to activate.
             */
            axisVectorMagnitude,

            /** \brief Use the value of the axis as-is, accounting for polarity/direction. */
            axisValue)

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
 * \brief Integral representation of \c InputType
 * \sa InputType
 */
using InputType_t = uint32_t;

/**
 * \brief The type of input to poll from the device.
 */
struct InputType
{
	enum T : InputType_t
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

ENUM_FLAGS(InputType);
ENUM_VALUES(InputType, 3);

using OutputType_t = uint32_t;

/**
 * \brief Bitfield representing an output simulation type.
 */
struct OutputType // TODO: vjoy output support
{
	enum T : OutputType_t
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

ENUM_FLAGS(OutputType);
ENUM_VALUES(OutputType, 3);

using XInputButtons_t = uint16_t;

/**
 * \brief Bitfield representing Xbox 360 digital buttons.
 */
struct XInputButtons
{
	enum T : XInputButtons_t
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

ENUM_FLAGS(XInputButtons);
ENUM_VALUES(XInputButtons, 16);

using XInputAxis_t = uint32_t;

/**
 * \brief Bitfield representing Xbox 360 axes.
 */
struct XInputAxis
{
	enum T : XInputAxis_t
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

ENUM_FLAGS(XInputAxis);
ENUM_VALUES(XInputAxis, 6);

using Direction_t = uint32_t;

/**
 * \brief Bitfield representing the direction of an action.
 */
struct Direction
{
	enum T : Direction_t
	{
		none,
		up    = 1 << 0,
		down  = 1 << 1,
		left  = 1 << 2,
		right = 1 << 3
	};
};

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

using Ds4Axes_t = uint32_t;

/**
 * \brief Bitfield representing the axes provided by the DualShock 4.
 */
struct Ds4Axes
{
	enum T : Ds4Axes_t
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

	static const Ds4Axes_t leftStick     = leftStickX | leftStickY;
	static const Ds4Axes_t rightStick    = rightStickX | rightStickY;
	static const Ds4Axes_t accelerometer = accelX | accelY | accelZ;
	static const Ds4Axes_t gyroscope     = gyroX | gyroY | gyroZ;

	/**
	 * \brief Given any combination of \c Ds4Axes bits, returns a
	 * bitmask expanded to cover all vector components of the axes.
	 * \param axes The axes to expand.
	 * \return The bitmask for expanded axes.
	 */
	static Ds4Axes_t expand(Ds4Axes_t axes);

	/**
	 * \brief Given an axis, returns the number of components in the vector it belongs to.
	 * \param vector_axis The axis to retrieve the vector component count of.
	 * \return The number of components in the axis vector.
	 */
	static size_t componentCount(Ds4Axes_t vector_axis);
};

ENUM_FLAGS(Ds4Axes);
ENUM_VALUES(Ds4Axes, 12);

using Ds4VectorInput_t = size_t;

struct Ds4VectorInput
{
	enum T : Ds4VectorInput_t
	{
		none,
		/** \brief The left stick of the controller. */
		leftStick,
		/** \brief The right stick of the controller. */
		rightStick,
		/** \brief The accelerometer. */
		accelerometer,
		/** \brief The gyroscope. */
		gyroscope
	};

	static T fromAxes(Ds4Axes_t axes);
};

using Ds4Extensions_t = uint8_t;

/**
 * \brief Bitfield representing the (known) extensions reported by the DualShock 4.
 */
struct Ds4Extensions
{
	enum T : Ds4Extensions_t
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

ENUM_FLAGS(Ds4Extensions);
ENUM_VALUES(Ds4Extensions, 4);
