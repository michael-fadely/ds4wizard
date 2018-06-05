#include "stdafx.h"
#include "enums.h"

#include <sstream>

#define SERIALIZE_DEF(TYPE)                                                    \
std::string serializeFlags_ ## TYPE (TYPE ## _t value)                          \
{                                                                               \
	bool empty = true;                                                          \
	std::stringstream ss;                                                       \
    size_t i = 0;                                                               \
                                                                                \
	for (auto bit : TYPE ## _bits)                                              \
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
				value |= TYPE ## _bits[i];                                      \
				break;                                                          \
			}                                                                   \
			++i;                                                                \
		}                                                                       \
	}                                                                           \
}

static const InputType_t InputType_bits[] = {
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

static const XInputAxis_t XInputAxis_bits[] = {
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

static const XInputButtons_t XInputButtons_bits[] = {
	XInputButtons::DPadUp,
	XInputButtons::DPadDown,
	XInputButtons::DPadLeft,
	XInputButtons::DPadRight,
	XInputButtons::Start,
	XInputButtons::Back,
	XInputButtons::LeftThumb,
	XInputButtons::RightThumb,
	XInputButtons::LeftShoulder,
	XInputButtons::RightShoulder,
	XInputButtons::Guide,
	XInputButtons::Dummy,
	XInputButtons::A,
	XInputButtons::B,
	XInputButtons::X,
	XInputButtons::Y
};

static const char* XInputButtons_names[] = {
	"DPadUp",
	"DPadDown",
	"DPadLeft",
	"DPadRight",
	"Start",
	"Back",
	"LeftThumb",
	"RightThumb",
	"LeftShoulder",
	"RightShoulder",
	"Guide",
	"Dummy",
	"A",
	"B",
	"X",
	"Y"
};

SERIALIZE_DEF(XInputButtons)

static const Ds4Buttons_t Ds4Buttons_bits[] = {
	Ds4Buttons::Square,
	Ds4Buttons::Cross,
	Ds4Buttons::Circle,
	Ds4Buttons::Triangle,
	Ds4Buttons::L1,
	Ds4Buttons::R1,
	Ds4Buttons::L2,
	Ds4Buttons::R2,
	Ds4Buttons::Share,
	Ds4Buttons::Options,
	Ds4Buttons::L3,
	Ds4Buttons::R3,
	Ds4Buttons::PS,
	Ds4Buttons::TouchButton,
	Ds4Buttons::Touch1,
	Ds4Buttons::Touch2,
	Ds4Buttons::Up,
	Ds4Buttons::Down,
	Ds4Buttons::Left,
	Ds4Buttons::Right
};

static const char* Ds4Buttons_names[] = {
	"Square",
	"Cross",
	"Circle",
	"Triangle",
	"L1",
	"R1",
	"L2",
	"R2",
	"Share",
	"Options",
	"L3",
	"R3",
	"PS",
	"TouchButton",
	"Touch1",
	"Touch2",
	"Up",
	"Down",
	"Left",
	"Right"
};

SERIALIZE_DEF(Ds4Buttons)

static const Ds4Axis_t Ds4Axis_bits[] = {
	Ds4Axis::LeftStickX,
	Ds4Axis::LeftStickY,
	Ds4Axis::RightStickX,
	Ds4Axis::RightStickY,
	Ds4Axis::LeftTrigger,
	Ds4Axis::RightTrigger,
	Ds4Axis::AccelX,
	Ds4Axis::AccelY,
	Ds4Axis::AccelZ,
	Ds4Axis::GyroX,
	Ds4Axis::GyroY,
	Ds4Axis::GyroZ,
};

static const char* Ds4Axis_names[] = {
	"leftStickX",
	"leftStickY",
	"rightStickX",
	"rightStickY",
	"leftTrigger",
	"rightTrigger",
	"AccelX",
	"AccelY",
	"AccelZ",
	"GyroX",
	"GyroY",
	"GyroZ"
};

SERIALIZE_DEF(Ds4Axis)

static const Ds4Extensions_t Ds4Extensions_bits[] = {
	Ds4Extensions::Cable,
	Ds4Extensions::Headphones,
	Ds4Extensions::Microphone,
	Ds4Extensions::Unknown
};

static const char* Ds4Extensions_names[] = {
	"Cable",
	"Headphones",
	"Microphone",
	"Unknown"
};

SERIALIZE_DEF(Ds4Extensions)

static const Direction_t Direction_bits[] = {
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

static const OutputType_t OutputType_bits[] = {
	OutputType::XInput,
	OutputType::Keyboard,
	OutputType::Mouse,
};

static const char* OutputType_names[] = {
	"XInput",
	"Keyboard",
	"Mouse"
};

SERIALIZE_DEF(OutputType)
