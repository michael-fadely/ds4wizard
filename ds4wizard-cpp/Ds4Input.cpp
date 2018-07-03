#include "stdafx.h"
#include "Ds4Input.h"

void Ds4Input::addButton(bool pressed, Ds4Buttons_t buttons)
{
	if (pressed)
	{
		heldButtons |= buttons;
	}
}

// TODO: just offset the buffer
void Ds4Input::update(const gsl::span<uint8_t>& buffer, size_t i)
{
	const Ds4Buttons_t lastDpad = heldButtons & (Ds4Buttons::up | Ds4Buttons::down | Ds4Buttons::left | Ds4Buttons::right);
	heldButtons = 0;

	data.leftStick.x       = buffer[i + 0];
	data.leftStick.y       = buffer[i + 1];
	data.rightStick.x      = buffer[i + 2];
	data.rightStick.y      = buffer[i + 3];
	data.dPad              = static_cast<Hat>(buffer[i + 4] & 0xF);
	data.square            = !!((buffer[i + 4] >> 4) & 1); // TODO: bitfield
	data.cross             = !!((buffer[i + 4] >> 5) & 1); // TODO: bitfield
	data.circle            = !!((buffer[i + 4] >> 6) & 1); // TODO: bitfield
	data.triangle          = !!((buffer[i + 4] >> 7) & 1); // TODO: bitfield
	data.l1                = !!((buffer[i + 5] >> 0) & 1); // TODO: bitfield
	data.r1                = !!((buffer[i + 5] >> 1) & 1); // TODO: bitfield
	data.l2                = !!((buffer[i + 5] >> 2) & 1); // TODO: bitfield
	data.r2                = !!((buffer[i + 5] >> 3) & 1); // TODO: bitfield
	data.share             = !!((buffer[i + 5] >> 4) & 1); // TODO: bitfield
	data.options           = !!((buffer[i + 5] >> 5) & 1); // TODO: bitfield
	data.l3                = !!((buffer[i + 5] >> 6) & 1); // TODO: bitfield
	data.r3                = !!((buffer[i + 5] >> 7) & 1); // TODO: bitfield
	data.ps                = !!((buffer[i + 6] >> 0) & 1); // TODO: bitfield
	data.touchButton       = !!((buffer[i + 6] >> 1) & 1); // TODO: bitfield
	data.frameCount        = static_cast<uint8_t>((buffer[i + 6] >> 2) & 0x3F);
	data.leftTrigger       = buffer[i + 7];
	data.rightTrigger      = buffer[i + 8];
	data.accel.x           = *reinterpret_cast<int16_t*>(&buffer[i + 12]);
	data.accel.y           = *reinterpret_cast<int16_t*>(&buffer[i + 14]);
	data.accel.z           = *reinterpret_cast<int16_t*>(&buffer[i + 16]);
	data.gyro.x            = *reinterpret_cast<int16_t*>(&buffer[i + 18]);
	data.gyro.y            = *reinterpret_cast<int16_t*>(&buffer[i + 20]);
	data.gyro.z            = *reinterpret_cast<int16_t*>(&buffer[i + 22]);
	data.extensions        = static_cast<uint8_t>(buffer[i + 29] >> 4);
	data.battery           = static_cast<uint8_t>(buffer[i + 29] & 0x0F);
	data.charging          = (data.extensions & Ds4Extensions::Cable) != 0 && data.battery <= 10;
	data.touchEvent        = static_cast<uint8_t>(buffer[i + 32] & 0x3F);
	data.touchFrame        = static_cast<uint8_t>(buffer[i + 33] & 0x3F);
	data.touch1            = !(buffer[i + 34] & 0x80);
	data.touch1Id          = static_cast<uint8_t>(buffer[i + 34] & 0x7F);
	data.touchPoint1.x     = static_cast<short>(*reinterpret_cast<int16_t*>(&buffer[i + 35]) & 0xFFF);
	data.touchPoint1.y     = static_cast<short>((*reinterpret_cast<int16_t*>(&buffer[i + 36]) >> 4) & 0xFFF);
	data.touch2            = !(buffer[i + 38] & 0x80);
	data.touch2Id          = static_cast<uint8_t>(buffer[i + 38] & 0x7F);
	data.touchPoint2.x     = static_cast<short>(*reinterpret_cast<int16_t*>(&buffer[i + 39]) & 0xFFF);
	data.touchPoint2.y     = static_cast<short>((*reinterpret_cast<int16_t*>(&buffer[i + 40]) >> 4) & 0xFFF);
	data.lastTouchPoint1.x = static_cast<short>(*reinterpret_cast<int16_t*>(&buffer[i + 43]) & 0xFFF);
	data.lastTouchPoint1.y = static_cast<short>((*reinterpret_cast<int16_t*>(&buffer[i + 44]) >> 4) & 0xFFF);
	data.lastTouchPoint2.x = static_cast<short>(*reinterpret_cast<int16_t*>(&buffer[i + 47]) & 0xFFF);
	data.lastTouchPoint2.y = static_cast<short>((*reinterpret_cast<int16_t*>(&buffer[i + 48]) >> 4) & 0xFFF);

	if ((data.extensions & Ds4Extensions::Cable) == 0)
	{
		++data.battery;
	}

	if (data.battery > 10)
	{
		data.battery = 10;
	}

	addButton(data.cross, Ds4Buttons::cross);
	addButton(data.circle, Ds4Buttons::circle);
	addButton(data.square, Ds4Buttons::square);
	addButton(data.triangle, Ds4Buttons::triangle);
	addButton(data.l1, Ds4Buttons::l1);
	addButton(data.r1, Ds4Buttons::r1);
	addButton(data.l2, Ds4Buttons::l2);
	addButton(data.r2, Ds4Buttons::r2);
	addButton(data.share, Ds4Buttons::share);
	addButton(data.options, Ds4Buttons::options);
	addButton(data.l3, Ds4Buttons::l3);
	addButton(data.r3, Ds4Buttons::r3);
	addButton(data.ps, Ds4Buttons::ps);
	addButton(data.touchButton, Ds4Buttons::touchButton);

	addButton(data.touch1, Ds4Buttons::touch1);
	addButton(data.touch2, Ds4Buttons::touch2);

	switch (data.dPad)
	{
		case Hat::north:
			addButton(true, Ds4Buttons::up);
			break;
		case Hat::northEast:
			addButton(true, Ds4Buttons::up | Ds4Buttons::right);
			break;
		case Hat::east:
			addButton(true, Ds4Buttons::right);
			break;
		case Hat::southEast:
			addButton(true, Ds4Buttons::right | Ds4Buttons::down);
			break;
		case Hat::south:
			addButton(true, Ds4Buttons::down);
			break;
		case Hat::southWest:
			addButton(true, Ds4Buttons::down | Ds4Buttons::left);
			break;
		case Hat::west:
			addButton(true, Ds4Buttons::left);
			break;
		case Hat::northWest:
			addButton(true, Ds4Buttons::left | Ds4Buttons::up);
			break;
		case Hat::none:
			break;
		default:
			addButton(true, lastDpad);
			break;
	}

	updateChangedState();
}

void Ds4Input::updateChangedState()
{
	releasedButtons = lastHeldButtons & (heldButtons ^ lastHeldButtons);
	pressedButtons  = heldButtons & (heldButtons ^ lastHeldButtons);
	lastHeldButtons = heldButtons;
	buttonsChanged  = releasedButtons != 0 || pressedButtons != 0;

	const Ds4Buttons_t touchMask = Ds4Buttons::touch1 | Ds4Buttons::touch2 | Ds4Buttons::touchButton;

	touchChanged = lastTouchFrame != data.touchFrame
	               || (pressedButtons & touchMask) != 0
	               || (releasedButtons & touchMask) != 0;

	lastTouchFrame = data.touchFrame;
}

void Ds4Input::toXInput(int index, std::unique_ptr<ScpDevice>& device)
{
	if (gamepad == lastGamepad)
	{
		return;
	}

	device->syncState(index, gamepad);
	lastGamepad = gamepad;
}

float Ds4Input::getAxis(Ds4Axis_t axis, std::optional<AxisPolarity> polarity) const
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
			result = data.leftTrigger / 255.0f;
			break;
		case Ds4Axis::rightTrigger:
			result = data.rightTrigger / 255.0f;
			break;

		case Ds4Axis::accelX:
			result = std::clamp(data.accel.x / (std::numeric_limits<short>::max() + 1.0f), 0.0f, 1.0f);
			break;
		case Ds4Axis::accelY:
			result = std::clamp(data.accel.y / (std::numeric_limits<short>::max() + 1.0f), 0.0f, 1.0f);
			break;
		case Ds4Axis::accelZ:
			result = std::clamp(data.accel.z / (std::numeric_limits<short>::max() + 1.0f), 0.0f, 1.0f);
			break;

		case Ds4Axis::gyroX:
			result = std::clamp(data.gyro.x / (std::numeric_limits<short>::max() + 1.0f), 0.0f, 1.0f);
			break;
		case Ds4Axis::gyroY:
			result = std::clamp(data.gyro.y / (std::numeric_limits<short>::max() + 1.0f), 0.0f, 1.0f);
			break;
		case Ds4Axis::gyroZ:
			result = std::clamp(data.gyro.z / (std::numeric_limits<short>::max() + 1.0f), 0.0f, 1.0f);
			break;

		default:
			throw /*new ArgumentOutOfRangeException(nameof(axis), axis, null) // TODO */;
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
