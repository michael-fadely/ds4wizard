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

const std::array<InputType_t, 3> InputType_values = {
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

const std::array<XInputAxis_t, 6> XInputAxis_values = {
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

const std::array<XInputButtons_t, 16> XInputButtons_values = {
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

const std::array<Ds4ButtonsRaw_t, 15> Ds4ButtonsRaw_values = {
	Ds4ButtonsRaw::hat,
	Ds4ButtonsRaw::square,
	Ds4ButtonsRaw::cross,
	Ds4ButtonsRaw::circle,
	Ds4ButtonsRaw::triangle,
	Ds4ButtonsRaw::l1,
	Ds4ButtonsRaw::r1,
	Ds4ButtonsRaw::l2,
	Ds4ButtonsRaw::r2,
	Ds4ButtonsRaw::share,
	Ds4ButtonsRaw::options,
	Ds4ButtonsRaw::l3,
	Ds4ButtonsRaw::r3,
	Ds4ButtonsRaw::ps,
	Ds4ButtonsRaw::touchButton,
};

static const char* Ds4ButtonsRaw_names[] = {
	"hat",
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
};

Hat Ds4ButtonsRaw::getHat(Ds4ButtonsRaw_t value)
{
	return static_cast<Hat>(value & hat_mask);
}

const std::array<Ds4Buttons_t, 20> Ds4Buttons_values = {
	Ds4Buttons::up,
	Ds4Buttons::down,
	Ds4Buttons::left,
	Ds4Buttons::right,
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
};

static const char* Ds4Buttons_names[] = {
	"up",
	"down",
	"left",
	"right",
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
};

Ds4Buttons_t Ds4Buttons::fromRaw(Ds4ButtonsRaw_t bits)
{
	const auto maskWithoutHat = Ds4ButtonsRaw::mask ^ Ds4ButtonsRaw::hat_mask;

	Ds4Buttons_t result = bits & maskWithoutHat;
	
	switch (Ds4ButtonsRaw::getHat(bits))
	{
		case Hat::north:
			result |= Ds4Buttons::up;
			break;
		case Hat::northEast:
			result |= Ds4Buttons::up | Ds4Buttons::right;
			break;
		case Hat::east:
			result |= Ds4Buttons::right;
			break;
		case Hat::southEast:
			result |= Ds4Buttons::right | Ds4Buttons::down;
			break;
		case Hat::south:
			result |= Ds4Buttons::down;
			break;
		case Hat::southWest:
			result |= Ds4Buttons::down | Ds4Buttons::left;
			break;
		case Hat::west:
			result |= Ds4Buttons::left;
			break;
		case Hat::northWest:
			result |= Ds4Buttons::left | Ds4Buttons::up;
			break;
		case Hat::none:
			break;
	}

	return result;
}

SERIALIZE_DEF(Ds4Buttons)

const std::array<Ds4Axes_t, 12> Ds4Axes_values = {
	Ds4Axes::leftStickX,
	Ds4Axes::leftStickY,
	Ds4Axes::rightStickX,
	Ds4Axes::rightStickY,
	Ds4Axes::leftTrigger,
	Ds4Axes::rightTrigger,
	Ds4Axes::accelX,
	Ds4Axes::accelY,
	Ds4Axes::accelZ,
	Ds4Axes::gyroX,
	Ds4Axes::gyroY,
	Ds4Axes::gyroZ,
};

static const char* Ds4Axes_names[] = {
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

SERIALIZE_DEF(Ds4Axes)

const std::array<Ds4Extensions_t, 4> Ds4Extensions_values = {
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

const std::array<Direction_t, 4> Direction_values = {
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

const std::array<OutputType_t, 3> OutputType_values = {
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
