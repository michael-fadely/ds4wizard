#include "stdafx.h"
#include "Ds4Input.h"

void Ds4Input::_addButton(uint8_t pressed, Ds4Buttons::T buttons)
{
	if (pressed != 0)
	{
		HeldButtons |= buttons;
	}
}

void Ds4Input::Update(gsl::span<uint8_t> buffer, int i)
{
	Ds4Buttons::T lastDpad = HeldButtons & (Ds4Buttons::Up | Ds4Buttons::Down | Ds4Buttons::Left | Ds4Buttons::Right);
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

	_addButton(Data.Cross, Ds4Buttons::Cross);
	_addButton(Data.Circle, Ds4Buttons::Circle);
	_addButton(Data.Square, Ds4Buttons::Square);
	_addButton(Data.Triangle, Ds4Buttons::Triangle);
	_addButton(Data.L1, Ds4Buttons::L1);
	_addButton(Data.R1, Ds4Buttons::R1);
	_addButton(Data.L2, Ds4Buttons::L2);
	_addButton(Data.R2, Ds4Buttons::R2);
	_addButton(Data.Share, Ds4Buttons::Share);
	_addButton(Data.Options, Ds4Buttons::Options);
	_addButton(Data.L3, Ds4Buttons::L3);
	_addButton(Data.R3, Ds4Buttons::R3);
	_addButton(Data.PS, Ds4Buttons::PS);
	_addButton(Data.TouchButton, Ds4Buttons::TouchButton);

	_addButton((uint8_t)(Data.Touch1 ? 1 : 0), Ds4Buttons::Touch1);
	_addButton((uint8_t)(Data.Touch2 ? 1 : 0), Ds4Buttons::Touch2);

	switch (Data.DPad)
	{
		case Hat::North:
			_addButton(1, Ds4Buttons::Up);
			break;
		case Hat::NorthEast:
			_addButton(1, Ds4Buttons::Up | Ds4Buttons::Right);
			break;
		case Hat::East:
			_addButton(1, Ds4Buttons::Right);
			break;
		case Hat::SouthEast:
			_addButton(1, Ds4Buttons::Right | Ds4Buttons::Down);
			break;
		case Hat::South:
			_addButton(1, Ds4Buttons::Down);
			break;
		case Hat::SouthWest:
			_addButton(1, Ds4Buttons::Down | Ds4Buttons::Left);
			break;
		case Hat::West:
			_addButton(1, Ds4Buttons::Left);
			break;
		case Hat::NorthWest:
			_addButton(1, Ds4Buttons::Left | Ds4Buttons::Up);
			break;
		case Hat::None:
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

	const Ds4Buttons::T touchMask = Ds4Buttons::Touch1 | Ds4Buttons::Touch2 | Ds4Buttons::TouchButton;

	TouchChanged = lastTouchFrame != Data.TouchFrame
	               || (PressedButtons & touchMask) != 0
	               || (ReleasedButtons & touchMask) != 0;

	lastTouchFrame = Data.TouchFrame;
}

void Ds4Input::ToXInput(int index, ScpDevice device)
{
	if (Gamepad == lastGamepad)
	{
		return;
	}

	device.SyncState(index, Gamepad);
	lastGamepad = Gamepad;
}

float Ds4Input::GetAxis(Ds4Axis axis, AxisPolarity polarity)
{
	float result;

	switch (axis)
	{
		case Ds4Axis::LeftStickX:
			result = Math.Min(1.0f, (Data.LeftStick.X - 128).Clamp(-127, 127) / 127.0f);
			result = result.Clamp(-1.0f, 1.0f);
			break;
		case Ds4Axis::LeftStickY:
			result = -Math.Min(1.0f, (Data.LeftStick.Y - 128).Clamp(-127, 127) / 127.0f);
			result = result.Clamp(-1.0f, 1.0f);
			break;

		case Ds4Axis::RightStickX:
			result = Math.Min(1.0f, (Data.RightStick.X - 128).Clamp(-127, 127) / 127.0f);
			result = result.Clamp(-1.0f, 1.0f);
			break;
		case Ds4Axis::RightStickY:
			result = -Math.Min(1.0f, (Data.RightStick.Y - 128).Clamp(-127, 127) / 127.0f);
			result = result.Clamp(-1.0f, 1.0f);
			break;

		case Ds4Axis::LeftTrigger:
			result = Data.LeftTrigger / 255.0f;
			break;
		case Ds4Axis::RightTrigger:
			result = Data.RightTrigger / 255.0f;
			break;

		case Ds4Axis::AccelX:
			result = (Data.Accel.X / (short.MaxValue + 1.0f)).Clamp(0.0f, 1.0f);
			break;
		case Ds4Axis::AccelY:
			result = (Data.Accel.Y / (short.MaxValue + 1.0f)).Clamp(0.0f, 1.0f);
			break;
		case Ds4Axis::AccelZ:
			result = (Data.Accel.Z / (short.MaxValue + 1.0f)).Clamp(0.0f, 1.0f);
			break;

		case Ds4Axis::GyroX:
			result = (Data.Gyro.X / (short.MaxValue + 1.0f)).Clamp(0.0f, 1.0f);
			break;
		case Ds4Axis::GyroY:
			result = (Data.Gyro.Y / (short.MaxValue + 1.0f)).Clamp(0.0f, 1.0f);
			break;
		case Ds4Axis::GyroZ:
			result = (Data.Gyro.Z / (short.MaxValue + 1.0f)).Clamp(0.0f, 1.0f);
			break;

		default:
			throw /*new ArgumentOutOfRangeException(nameof(axis), axis, null) // TODO */;
	}

	if (!polarity.HasValue)
	{
		return result;
	}

	if (polarity == AxisPolarity::Negative)
	{
		result = -result;
	}

	return Math.Max(0.0f, result);
}
