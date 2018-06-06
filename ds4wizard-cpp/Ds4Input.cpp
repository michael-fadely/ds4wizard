#include "stdafx.h"
#include "Ds4Input.h"

void Ds4Input::_addButton(uint8_t pressed, Ds4Buttons_t buttons)
{
	if (pressed != 0)
	{
		HeldButtons |= buttons;
	}
}

void Ds4Input::Update(const gsl::span<uint8_t>& buffer, int i)
{
	const Ds4Buttons_t lastDpad = HeldButtons & (Ds4Buttons::up | Ds4Buttons::down | Ds4Buttons::left | Ds4Buttons::right);
	HeldButtons = 0;

	Data.LeftStick.X       = buffer[i + 0];
	Data.LeftStick.Y       = buffer[i + 1];
	Data.RightStick.X      = buffer[i + 2];
	Data.RightStick.Y      = buffer[i + 3];
	Data.DPad              = (Hat)(buffer[i + 4] & 0xF);
	Data.Square            = (uint8_t)((buffer[i + 4] >> 4) & 1); // TODO: bitfield
	Data.Cross             = (uint8_t)((buffer[i + 4] >> 5) & 1); // TODO: bitfield
	Data.Circle            = (uint8_t)((buffer[i + 4] >> 6) & 1); // TODO: bitfield
	Data.Triangle          = (uint8_t)((buffer[i + 4] >> 7) & 1); // TODO: bitfield
	Data.L1                = (uint8_t)((buffer[i + 5] >> 0) & 1); // TODO: bitfield
	Data.R1                = (uint8_t)((buffer[i + 5] >> 1) & 1); // TODO: bitfield
	Data.L2                = (uint8_t)((buffer[i + 5] >> 2) & 1); // TODO: bitfield
	Data.R2                = (uint8_t)((buffer[i + 5] >> 3) & 1); // TODO: bitfield
	Data.Share             = (uint8_t)((buffer[i + 5] >> 4) & 1); // TODO: bitfield
	Data.Options           = (uint8_t)((buffer[i + 5] >> 5) & 1); // TODO: bitfield
	Data.L3                = (uint8_t)((buffer[i + 5] >> 6) & 1); // TODO: bitfield
	Data.R3                = (uint8_t)((buffer[i + 5] >> 7) & 1); // TODO: bitfield
	Data.PS                = (uint8_t)((buffer[i + 6] >> 0) & 1); // TODO: bitfield
	Data.TouchButton       = (uint8_t)((buffer[i + 6] >> 1) & 1); // TODO: bitfield
	Data.FrameCount        = (uint8_t)((buffer[i + 6] >> 2) & 0x3F);
	Data.LeftTrigger       = buffer[i + 7];
	Data.RightTrigger      = buffer[i + 8];
	Data.Accel.X           = *reinterpret_cast<int16_t*>(&buffer[i + 12]);
	Data.Accel.Y           = *reinterpret_cast<int16_t*>(&buffer[i + 14]);
	Data.Accel.Z           = *reinterpret_cast<int16_t*>(&buffer[i + 16]);
	Data.Gyro.X            = *reinterpret_cast<int16_t*>(&buffer[i + 18]);
	Data.Gyro.Y            = *reinterpret_cast<int16_t*>(&buffer[i + 20]);
	Data.Gyro.Z            = *reinterpret_cast<int16_t*>(&buffer[i + 22]);
	Data.Extensions        = (uint8_t)(buffer[i + 29] >> 4);
	Data.Battery           = (uint8_t)(buffer[i + 29] & 0x0F);
	Data.Charging          = (Data.Extensions & Ds4Extensions::Cable) != 0 && Data.Battery <= 10;
	Data.TouchEvent        = (uint8_t)(buffer[i + 32] & 0x3F);
	Data.TouchFrame        = (uint8_t)(buffer[i + 33] & 0x3F);
	Data.Touch1            = (buffer[i + 34] & 0x80) == 0;
	Data.Touch1Id          = (uint8_t)(buffer[i + 34] & 0x7F);
	Data.TouchPoint1.X     = (short)(*reinterpret_cast<int16_t*>(&buffer[i + 35]) & 0xFFF);
	Data.TouchPoint1.Y     = (short)((*reinterpret_cast<int16_t*>(&buffer[i + 36]) >> 4) & 0xFFF);
	Data.Touch2            = (buffer[i + 38] & 0x80) == 0;
	Data.Touch2Id          = (uint8_t)(buffer[i + 38] & 0x7F);
	Data.TouchPoint2.X     = (short)(*reinterpret_cast<int16_t*>(&buffer[i + 39]) & 0xFFF);
	Data.TouchPoint2.Y     = (short)((*reinterpret_cast<int16_t*>(&buffer[i + 40]) >> 4) & 0xFFF);
	Data.LastTouchPoint1.X = (short)(*reinterpret_cast<int16_t*>(&buffer[i + 43]) & 0xFFF);
	Data.LastTouchPoint1.Y = (short)((*reinterpret_cast<int16_t*>(&buffer[i + 44]) >> 4) & 0xFFF);
	Data.LastTouchPoint2.X = (short)(*reinterpret_cast<int16_t*>(&buffer[i + 47]) & 0xFFF);
	Data.LastTouchPoint2.Y = (short)((*reinterpret_cast<int16_t*>(&buffer[i + 48]) >> 4) & 0xFFF);

	if ((Data.Extensions & Ds4Extensions::Cable) == 0)
	{
		++Data.Battery;
	}

	if (Data.Battery > 10)
	{
		Data.Battery = 10;
	}

	_addButton(Data.Cross, Ds4Buttons::cross);
	_addButton(Data.Circle, Ds4Buttons::circle);
	_addButton(Data.Square, Ds4Buttons::square);
	_addButton(Data.Triangle, Ds4Buttons::triangle);
	_addButton(Data.L1, Ds4Buttons::l1);
	_addButton(Data.R1, Ds4Buttons::r1);
	_addButton(Data.L2, Ds4Buttons::l2);
	_addButton(Data.R2, Ds4Buttons::r2);
	_addButton(Data.Share, Ds4Buttons::share);
	_addButton(Data.Options, Ds4Buttons::options);
	_addButton(Data.L3, Ds4Buttons::l3);
	_addButton(Data.R3, Ds4Buttons::r3);
	_addButton(Data.PS, Ds4Buttons::ps);
	_addButton(Data.TouchButton, Ds4Buttons::touchButton);

	_addButton((uint8_t)(Data.Touch1 ? 1 : 0), Ds4Buttons::touch1);
	_addButton((uint8_t)(Data.Touch2 ? 1 : 0), Ds4Buttons::touch2);

	switch (Data.DPad)
	{
		case Hat::north:
			_addButton(1, Ds4Buttons::up);
			break;
		case Hat::northEast:
			_addButton(1, Ds4Buttons::up | Ds4Buttons::right);
			break;
		case Hat::east:
			_addButton(1, Ds4Buttons::right);
			break;
		case Hat::southEast:
			_addButton(1, Ds4Buttons::right | Ds4Buttons::down);
			break;
		case Hat::south:
			_addButton(1, Ds4Buttons::down);
			break;
		case Hat::southWest:
			_addButton(1, Ds4Buttons::down | Ds4Buttons::left);
			break;
		case Hat::west:
			_addButton(1, Ds4Buttons::left);
			break;
		case Hat::northWest:
			_addButton(1, Ds4Buttons::left | Ds4Buttons::up);
			break;
		case Hat::none:
			break;
		default:
			_addButton(1, lastDpad);
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

	TouchChanged = lastTouchFrame != Data.TouchFrame
	               || (PressedButtons & touchMask) != 0
	               || (ReleasedButtons & touchMask) != 0;

	lastTouchFrame = Data.TouchFrame;
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

float Ds4Input::GetAxis(Ds4Axis_t axis, AxisPolarity* polarity) const
{
	float result;

	switch (axis)
	{
		case Ds4Axis::leftStickX:
			result = std::min(1.0f, std::clamp(Data.LeftStick.X - 128, -127, 127) / 127.0f);
			result = std::clamp(result, -1.0f, 1.0f);
			break;
		case Ds4Axis::leftStickY:
			result = -std::min(1.0f, std::clamp(Data.LeftStick.Y - 128, -127, 127) / 127.0f);
			result = std::clamp(result, -1.0f, 1.0f);
			break;

		case Ds4Axis::rightStickX:
			result = std::min(1.0f, std::clamp(Data.RightStick.X - 128, -127, 127) / 127.0f);
			result = std::clamp(result, -1.0f, 1.0f);
			break;
		case Ds4Axis::rightStickY:
			result = -std::min(1.0f, std::clamp(Data.RightStick.Y - 128, -127, 127) / 127.0f);
			result = std::clamp(result, -1.0f, 1.0f);
			break;

		case Ds4Axis::leftTrigger:
			result = Data.LeftTrigger / 255.0f;
			break;
		case Ds4Axis::rightTrigger:
			result = Data.RightTrigger / 255.0f;
			break;

		case Ds4Axis::accelX:
			result = std::clamp(Data.Accel.X / (std::numeric_limits<short>::max() + 1.0f), 0.0f, 1.0f);
			break;
		case Ds4Axis::accelY:
			result = std::clamp(Data.Accel.Y / (std::numeric_limits<short>::max() + 1.0f), 0.0f, 1.0f);
			break;
		case Ds4Axis::accelZ:
			result = std::clamp(Data.Accel.Z / (std::numeric_limits<short>::max() + 1.0f), 0.0f, 1.0f);
			break;

		case Ds4Axis::gyroX:
			result = std::clamp(Data.Gyro.X / (std::numeric_limits<short>::max() + 1.0f), 0.0f, 1.0f);
			break;
		case Ds4Axis::gyroY:
			result = std::clamp(Data.Gyro.Y / (std::numeric_limits<short>::max() + 1.0f), 0.0f, 1.0f);
			break;
		case Ds4Axis::gyroZ:
			result = std::clamp(Data.Gyro.Z / (std::numeric_limits<short>::max() + 1.0f), 0.0f, 1.0f);
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
