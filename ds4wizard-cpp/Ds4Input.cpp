#include "stdafx.h"
#include "Ds4Input.h"

void Ds4Input::addButton(bool pressed, Ds4Buttons_t buttons)
{
	if (pressed)
	{
		HeldButtons |= buttons;
	}
}

void Ds4Input::Update(const gsl::span<uint8_t>& buffer, int i)
{
	const Ds4Buttons_t lastDpad = HeldButtons & (Ds4Buttons::up | Ds4Buttons::down | Ds4Buttons::left | Ds4Buttons::right);
	HeldButtons = 0;

	Data.leftStick.x       = buffer[i + 0];
	Data.leftStick.y       = buffer[i + 1];
	Data.rightStick.x      = buffer[i + 2];
	Data.rightStick.y      = buffer[i + 3];
	Data.dPad              = static_cast<Hat>(buffer[i + 4] & 0xF);
	Data.square            = !!((buffer[i + 4] >> 4) & 1); // TODO: bitfield
	Data.cross             = !!((buffer[i + 4] >> 5) & 1); // TODO: bitfield
	Data.circle            = !!((buffer[i + 4] >> 6) & 1); // TODO: bitfield
	Data.triangle          = !!((buffer[i + 4] >> 7) & 1); // TODO: bitfield
	Data.l1                = !!((buffer[i + 5] >> 0) & 1); // TODO: bitfield
	Data.r1                = !!((buffer[i + 5] >> 1) & 1); // TODO: bitfield
	Data.l2                = !!((buffer[i + 5] >> 2) & 1); // TODO: bitfield
	Data.r2                = !!((buffer[i + 5] >> 3) & 1); // TODO: bitfield
	Data.share             = !!((buffer[i + 5] >> 4) & 1); // TODO: bitfield
	Data.options           = !!((buffer[i + 5] >> 5) & 1); // TODO: bitfield
	Data.l3                = !!((buffer[i + 5] >> 6) & 1); // TODO: bitfield
	Data.r3                = !!((buffer[i + 5] >> 7) & 1); // TODO: bitfield
	Data.ps                = !!((buffer[i + 6] >> 0) & 1); // TODO: bitfield
	Data.touchButton       = !!((buffer[i + 6] >> 1) & 1); // TODO: bitfield
	Data.frameCount        = static_cast<uint8_t>((buffer[i + 6] >> 2) & 0x3F);
	Data.leftTrigger       = buffer[i + 7];
	Data.rightTrigger      = buffer[i + 8];
	Data.accel.x           = *reinterpret_cast<int16_t*>(&buffer[i + 12]);
	Data.accel.y           = *reinterpret_cast<int16_t*>(&buffer[i + 14]);
	Data.accel.z           = *reinterpret_cast<int16_t*>(&buffer[i + 16]);
	Data.gyro.x            = *reinterpret_cast<int16_t*>(&buffer[i + 18]);
	Data.gyro.y            = *reinterpret_cast<int16_t*>(&buffer[i + 20]);
	Data.gyro.z            = *reinterpret_cast<int16_t*>(&buffer[i + 22]);
	Data.extensions        = static_cast<uint8_t>(buffer[i + 29] >> 4);
	Data.battery           = static_cast<uint8_t>(buffer[i + 29] & 0x0F);
	Data.charging          = (Data.extensions & Ds4Extensions::Cable) != 0 && Data.battery <= 10;
	Data.touchEvent        = static_cast<uint8_t>(buffer[i + 32] & 0x3F);
	Data.touchFrame        = static_cast<uint8_t>(buffer[i + 33] & 0x3F);
	Data.touch1            = !(buffer[i + 34] & 0x80);
	Data.touch1Id          = static_cast<uint8_t>(buffer[i + 34] & 0x7F);
	Data.touchPoint1.x     = static_cast<short>(*reinterpret_cast<int16_t*>(&buffer[i + 35]) & 0xFFF);
	Data.touchPoint1.y     = static_cast<short>((*reinterpret_cast<int16_t*>(&buffer[i + 36]) >> 4) & 0xFFF);
	Data.touch2            = !(buffer[i + 38] & 0x80);
	Data.touch2Id          = static_cast<uint8_t>(buffer[i + 38] & 0x7F);
	Data.touchPoint2.x     = static_cast<short>(*reinterpret_cast<int16_t*>(&buffer[i + 39]) & 0xFFF);
	Data.touchPoint2.y     = static_cast<short>((*reinterpret_cast<int16_t*>(&buffer[i + 40]) >> 4) & 0xFFF);
	Data.lastTouchPoint1.x = static_cast<short>(*reinterpret_cast<int16_t*>(&buffer[i + 43]) & 0xFFF);
	Data.lastTouchPoint1.y = static_cast<short>((*reinterpret_cast<int16_t*>(&buffer[i + 44]) >> 4) & 0xFFF);
	Data.lastTouchPoint2.x = static_cast<short>(*reinterpret_cast<int16_t*>(&buffer[i + 47]) & 0xFFF);
	Data.lastTouchPoint2.y = static_cast<short>((*reinterpret_cast<int16_t*>(&buffer[i + 48]) >> 4) & 0xFFF);

	if ((Data.extensions & Ds4Extensions::Cable) == 0)
	{
		++Data.battery;
	}

	if (Data.battery > 10)
	{
		Data.battery = 10;
	}

	addButton(Data.cross, Ds4Buttons::cross);
	addButton(Data.circle, Ds4Buttons::circle);
	addButton(Data.square, Ds4Buttons::square);
	addButton(Data.triangle, Ds4Buttons::triangle);
	addButton(Data.l1, Ds4Buttons::l1);
	addButton(Data.r1, Ds4Buttons::r1);
	addButton(Data.l2, Ds4Buttons::l2);
	addButton(Data.r2, Ds4Buttons::r2);
	addButton(Data.share, Ds4Buttons::share);
	addButton(Data.options, Ds4Buttons::options);
	addButton(Data.l3, Ds4Buttons::l3);
	addButton(Data.r3, Ds4Buttons::r3);
	addButton(Data.ps, Ds4Buttons::ps);
	addButton(Data.touchButton, Ds4Buttons::touchButton);

	addButton(Data.touch1, Ds4Buttons::touch1);
	addButton(Data.touch2, Ds4Buttons::touch2);

	switch (Data.dPad)
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

	UpdateChangedState();
}

void Ds4Input::UpdateChangedState()
{
	ReleasedButtons = LastHeldButtons & (HeldButtons ^ LastHeldButtons);
	PressedButtons  = HeldButtons & (HeldButtons ^ LastHeldButtons);
	LastHeldButtons = HeldButtons;
	ButtonsChanged  = ReleasedButtons != 0 || PressedButtons != 0;

	const Ds4Buttons_t touchMask = Ds4Buttons::touch1 | Ds4Buttons::touch2 | Ds4Buttons::touchButton;

	TouchChanged = lastTouchFrame != Data.touchFrame
	               || (PressedButtons & touchMask) != 0
	               || (ReleasedButtons & touchMask) != 0;

	lastTouchFrame = Data.touchFrame;
}

void Ds4Input::ToXInput(int index, std::unique_ptr<ScpDevice>& device)
{
	if (Gamepad == lastGamepad)
	{
		return;
	}

	device->SyncState(index, Gamepad);
	lastGamepad = Gamepad;
}

float Ds4Input::GetAxis(Ds4Axis_t axis, std::optional<AxisPolarity> polarity) const
{
	float result;

	switch (axis)
	{
		case Ds4Axis::leftStickX:
			result = std::min(1.0f, std::clamp(Data.leftStick.x - 128, -127, 127) / 127.0f);
			result = std::clamp(result, -1.0f, 1.0f);
			break;
		case Ds4Axis::leftStickY:
			result = -std::min(1.0f, std::clamp(Data.leftStick.y - 128, -127, 127) / 127.0f);
			result = std::clamp(result, -1.0f, 1.0f);
			break;

		case Ds4Axis::rightStickX:
			result = std::min(1.0f, std::clamp(Data.rightStick.x - 128, -127, 127) / 127.0f);
			result = std::clamp(result, -1.0f, 1.0f);
			break;
		case Ds4Axis::rightStickY:
			result = -std::min(1.0f, std::clamp(Data.rightStick.y - 128, -127, 127) / 127.0f);
			result = std::clamp(result, -1.0f, 1.0f);
			break;

		case Ds4Axis::leftTrigger:
			result = Data.leftTrigger / 255.0f;
			break;
		case Ds4Axis::rightTrigger:
			result = Data.rightTrigger / 255.0f;
			break;

		case Ds4Axis::accelX:
			result = std::clamp(Data.accel.x / (std::numeric_limits<short>::max() + 1.0f), 0.0f, 1.0f);
			break;
		case Ds4Axis::accelY:
			result = std::clamp(Data.accel.y / (std::numeric_limits<short>::max() + 1.0f), 0.0f, 1.0f);
			break;
		case Ds4Axis::accelZ:
			result = std::clamp(Data.accel.z / (std::numeric_limits<short>::max() + 1.0f), 0.0f, 1.0f);
			break;

		case Ds4Axis::gyroX:
			result = std::clamp(Data.gyro.x / (std::numeric_limits<short>::max() + 1.0f), 0.0f, 1.0f);
			break;
		case Ds4Axis::gyroY:
			result = std::clamp(Data.gyro.y / (std::numeric_limits<short>::max() + 1.0f), 0.0f, 1.0f);
			break;
		case Ds4Axis::gyroZ:
			result = std::clamp(Data.gyro.z / (std::numeric_limits<short>::max() + 1.0f), 0.0f, 1.0f);
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
