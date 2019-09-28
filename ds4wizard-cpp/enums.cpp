#include "pch.h"
#include "enums.h"

#include <sstream>

#define SERIALIZE_DEF(TYPE)                                                     \
std::string serializeFlags_ ## TYPE (TYPE ## _t value)                          \
{                                                                               \
	bool empty = true;                                                          \
	std::stringstream ss;                                                       \
    size_t i = 0;                                                               \
                                                                                \
	for (auto bit : TYPE ## _values)                                            \
	{                                                                           \
		if (value & bit)                                                        \
		{                                                                       \
			if (!empty)                                                         \
			{                                                                   \
				ss << '|';                                                      \
			}                                                                   \
                                                                                \
			empty = false;                                                      \
			ss << TYPE ## _names[i];                                            \
		}                                                                       \
		++i;                                                                    \
	}                                                                           \
                                                                                \
	return ss.str();                                                            \
}                                                                               \
                                                                                \
void deserializeFlags_ ## TYPE ## (const std::string& input, TYPE ## _t& value) \
{                                                                               \
	std::stringstream ss;                                                       \
	ss << input;                                                                \
                                                                                \
	value = 0;                                                                  \
                                                                                \
	for (std::string l; std::getline(ss, l, '|');)                              \
	{                                                                           \
		size_t i = 0;                                                           \
		for (const auto& name : TYPE ## _names)                                 \
		{                                                                       \
			if (name == l)                                                      \
			{                                                                   \
				value |= TYPE ## _values[i];                                    \
				break;                                                          \
			}                                                                   \
			++i;                                                                \
		}                                                                       \
	}                                                                           \
}

static const InputType_t InputType_values[] = {
	InputType::button,
	InputType::axis,
	InputType::touchRegion
};

static const char* InputType_names[] = {
	"button",
	"axis",
	"touchRegion"
};

SERIALIZE_DEF(InputType)

static const XInputAxis_t XInputAxis_values[] = {
	XInputAxis::leftStickX,
	XInputAxis::leftStickY,
	XInputAxis::rightStickX,
	XInputAxis::rightStickY,
	XInputAxis::leftTrigger,
	XInputAxis::rightTrigger
};

static const char* XInputAxis_names[] = {
	"leftStickX",
	"leftStickY",
	"rightStickX",
	"rightStickY",
	"leftTrigger",
	"rightTrigger"
};

SERIALIZE_DEF(XInputAxis)

static const XInputButtons_t XInputButtons_values[] = {
	XInputButtons::dPadUp,
	XInputButtons::dPadDown,
	XInputButtons::dPadLeft,
	XInputButtons::dPadRight,
	XInputButtons::start,
	XInputButtons::back,
	XInputButtons::leftThumb,
	XInputButtons::rightThumb,
	XInputButtons::leftShoulder,
	XInputButtons::rightShoulder,
	XInputButtons::guide,
	XInputButtons::dummy,
	XInputButtons::a,
	XInputButtons::b,
	XInputButtons::x,
	XInputButtons::y
};

static const char* XInputButtons_names[] = {
	"dPadUp",
	"dPadDown",
	"dPadLeft",
	"dPadRight",
	"start",
	"back",
	"leftThumb",
	"rightThumb",
	"leftShoulder",
	"rightShoulder",
	"guide",
	"dummy",
	"a",
	"b",
	"x",
	"y"
};

SERIALIZE_DEF(XInputButtons)

static const Ds4Buttons_t Ds4Buttons_values[] = {
	Ds4Buttons::square,
	Ds4Buttons::cross,
	Ds4Buttons::circle,
	Ds4Buttons::triangle,
	Ds4Buttons::l1,
	Ds4Buttons::r1,
	Ds4Buttons::l2,
	Ds4Buttons::r2,
	Ds4Buttons::share,
	Ds4Buttons::options,
	Ds4Buttons::l3,
	Ds4Buttons::r3,
	Ds4Buttons::ps,
	Ds4Buttons::touchButton,
	Ds4Buttons::touch1,
	Ds4Buttons::touch2,
	Ds4Buttons::up,
	Ds4Buttons::down,
	Ds4Buttons::left,
	Ds4Buttons::right
};

static const char* Ds4Buttons_names[] = {
	"square",
	"cross",
	"circle",
	"triangle",
	"l1",
	"r1",
	"l2",
	"r2",
	"share",
	"options",
	"l3",
	"r3",
	"ps",
	"touchButton",
	"touch1",
	"touch2",
	"up",
	"down",
	"left",
	"right"
};

SERIALIZE_DEF(Ds4Buttons)

static const Ds4Axis_t Ds4Axis_values[] = {
	Ds4Axis::leftStickX,
	Ds4Axis::leftStickY,
	Ds4Axis::rightStickX,
	Ds4Axis::rightStickY,
	Ds4Axis::leftTrigger,
	Ds4Axis::rightTrigger,
	Ds4Axis::accelX,
	Ds4Axis::accelY,
	Ds4Axis::accelZ,
	Ds4Axis::gyroX,
	Ds4Axis::gyroY,
	Ds4Axis::gyroZ,
};

static const char* Ds4Axis_names[] = {
	"leftStickX",
	"leftStickY",
	"rightStickX",
	"rightStickY",
	"leftTrigger",
	"rightTrigger",
	"accelX",
	"accelY",
	"accelZ",
	"gyroX",
	"gyroY",
	"gyroZ"
};

SERIALIZE_DEF(Ds4Axis)

static const Ds4Extensions_t Ds4Extensions_values[] = {
	Ds4Extensions::cable,
	Ds4Extensions::headphones,
	Ds4Extensions::microphone,
	Ds4Extensions::unknown
};

static const char* Ds4Extensions_names[] = {
	"cable",
	"headphones",
	"microphone",
	"unknown"
};

SERIALIZE_DEF(Ds4Extensions)

static const Direction_t Direction_values[] = {
	Direction::up,
	Direction::down,
	Direction::left,
	Direction::right
};

static const char* Direction_names[] = {
	"up",
	"down",
	"left",
	"right",
};

SERIALIZE_DEF(Direction)

static const OutputType_t OutputType_values[] = {
	OutputType::xinput,
	OutputType::keyboard,
	OutputType::mouse,
};

static const char* OutputType_names[] = {
	"xinput",
	"keyboard",
	"mouse"
};

SERIALIZE_DEF(OutputType)
