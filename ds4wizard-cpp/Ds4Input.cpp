#include "pch.h"
#include "Ds4Input.h"

inline void Ds4Input::addButton(bool pressed, Ds4Buttons_t buttons)
{
	if (pressed)
	{
		heldButtons |= buttons;
	}
}

void Ds4Input::updateButtons()
{
	const Ds4Buttons_t lastDpad = heldButtons & Ds4Buttons::dpad;
	heldButtons = Ds4Buttons::fromRaw(data.activeButtons);

	addButton(data.touch1, Ds4Buttons::touch1);
	addButton(data.touch2, Ds4Buttons::touch2);

	// check if the hat switch is valid, otherwise use last hat switch
	switch (data.dPad())
	{
		case Hat::north:
		case Hat::northEast:
		case Hat::east:
		case Hat::southEast:
		case Hat::south:
		case Hat::southWest:
		case Hat::west:
		case Hat::northWest:
		case Hat::none:
			break;

		default:
			heldButtons |= lastDpad;
			break;
	}
}

void Ds4Input::update(const gsl::span<uint8_t>& buffer)
{
	data.leftStick.x       = buffer[0];
	data.leftStick.y       = buffer[1];
	data.rightStick.x      = buffer[2];
	data.rightStick.y      = buffer[3];
	data.frameCount        = static_cast<uint8_t>((buffer[6] >> 2) & 0x3F);
	data.leftTrigger       = buffer[7];
	data.rightTrigger      = buffer[8];
	data.accel.x           = *reinterpret_cast<int16_t*>(&buffer[12]);
	data.accel.y           = *reinterpret_cast<int16_t*>(&buffer[14]);
	data.accel.z           = *reinterpret_cast<int16_t*>(&buffer[16]);
	data.gyro.x            = *reinterpret_cast<int16_t*>(&buffer[18]);
	data.gyro.y            = *reinterpret_cast<int16_t*>(&buffer[20]);
	data.gyro.z            = *reinterpret_cast<int16_t*>(&buffer[22]);
	data.extensions        = static_cast<uint8_t>(buffer[29] >> 4);
	data.battery           = static_cast<uint8_t>(buffer[29] & 0x0F);
	data.touchEvent        = static_cast<uint8_t>(buffer[32] & 0x3F);
	data.touchFrame        = static_cast<uint8_t>(buffer[33] & 0x3F);
	data.touch1            = !(buffer[34] & 0x80);
	data.touch1Id          = static_cast<uint8_t>(buffer[34] & 0x7F);
	data.touchPoint1.x     = static_cast<short>(*reinterpret_cast<int16_t*>(&buffer[35]) & 0xFFF);
	data.touchPoint1.y     = static_cast<short>((*reinterpret_cast<int16_t*>(&buffer[36]) >> 4) & 0xFFF);
	data.touch2            = !(buffer[38] & 0x80);
	data.touch2Id          = static_cast<uint8_t>(buffer[38] & 0x7F);
	data.touchPoint2.x     = static_cast<short>(*reinterpret_cast<int16_t*>(&buffer[39]) & 0xFFF);
	data.touchPoint2.y     = static_cast<short>((*reinterpret_cast<int16_t*>(&buffer[40]) >> 4) & 0xFFF);
	data.lastTouchPoint1.x = static_cast<short>(*reinterpret_cast<int16_t*>(&buffer[43]) & 0xFFF);
	data.lastTouchPoint1.y = static_cast<short>((*reinterpret_cast<int16_t*>(&buffer[44]) >> 4) & 0xFFF);
	data.lastTouchPoint2.x = static_cast<short>(*reinterpret_cast<int16_t*>(&buffer[47]) & 0xFFF);
	data.lastTouchPoint2.y = static_cast<short>((*reinterpret_cast<int16_t*>(&buffer[48]) >> 4) & 0xFFF);

	if (!(data.extensions & Ds4Extensions::cable))
	{
		++data.battery;
	}

	if (data.battery > 10)
	{
		data.battery = 10;
	}

	data.activeButtons = *reinterpret_cast<Ds4ButtonsRaw_t*>(&buffer[4]);

	updateButtons();

	updateChangedState();
}

void Ds4Input::updateChangedState()
{
	releasedButtons = lastHeldButtons & (heldButtons ^ lastHeldButtons);
	pressedButtons  = heldButtons & (heldButtons ^ lastHeldButtons);
	lastHeldButtons = heldButtons;
	buttonsChanged  = !!releasedButtons || !!pressedButtons;

	constexpr Ds4Buttons_t touchMask = Ds4Buttons::touch1 | Ds4Buttons::touch2 | Ds4Buttons::touchButton;

	touchChanged = lastTouchFrame != data.touchFrame ||
	               (pressedButtons & touchMask) != 0 ||
	               (releasedButtons & touchMask) != 0;

	lastTouchFrame = data.touchFrame;
}

float Ds4Input::getAxis(Ds4Axis_t axis, const std::optional<AxisPolarity>& polarity) const
{
	float result;

	switch (axis)
	{
		case Ds4Axis::leftStickX:
			result = std::min(1.0f, std::clamp(data.leftStick.x - 128, -127, 127) / 127.0f);
			result = std::clamp(result, -1.0f, 1.0f);
			break;
		case Ds4Axis::leftStickY:
			result = -std::min(1.0f, std::clamp(data.leftStick.y - 128, -127, 127) / 127.0f);
			result = std::clamp(result, -1.0f, 1.0f);
			break;

		case Ds4Axis::rightStickX:
			result = std::min(1.0f, std::clamp(data.rightStick.x - 128, -127, 127) / 127.0f);
			result = std::clamp(result, -1.0f, 1.0f);
			break;
		case Ds4Axis::rightStickY:
			result = -std::min(1.0f, std::clamp(data.rightStick.y - 128, -127, 127) / 127.0f);
			result = std::clamp(result, -1.0f, 1.0f);
			break;

		case Ds4Axis::leftTrigger:
			result = static_cast<float>(data.leftTrigger) / 255.0f;
			break;
		case Ds4Axis::rightTrigger:
			result = static_cast<float>(data.rightTrigger) / 255.0f;
			break;

		case Ds4Axis::accelX:
			result = std::clamp(static_cast<float>(data.accel.x) / (std::numeric_limits<short>::max() + 1.0f), 0.0f, 1.0f);
			break;
		case Ds4Axis::accelY:
			result = std::clamp(static_cast<float>(data.accel.y) / (std::numeric_limits<short>::max() + 1.0f), 0.0f, 1.0f);
			break;
		case Ds4Axis::accelZ:
			result = std::clamp(static_cast<float>(data.accel.z) / (std::numeric_limits<short>::max() + 1.0f), 0.0f, 1.0f);
			break;

		case Ds4Axis::gyroX:
			result = std::clamp(static_cast<float>(data.gyro.x) / (std::numeric_limits<short>::max() + 1.0f), 0.0f, 1.0f);
			break;
		case Ds4Axis::gyroY:
			result = std::clamp(static_cast<float>(data.gyro.y) / (std::numeric_limits<short>::max() + 1.0f), 0.0f, 1.0f);
			break;
		case Ds4Axis::gyroZ:
			result = std::clamp(static_cast<float>(data.gyro.z) / (std::numeric_limits<short>::max() + 1.0f), 0.0f, 1.0f);
			break;

		default:
			throw std::out_of_range("invalid Ds4Axis");
	}

	if (!polarity)
	{
		return result;
	}

	if (*polarity == +AxisPolarity::negative)
	{
		result = -result;
	}

	return std::max(0.0f, result);
}
