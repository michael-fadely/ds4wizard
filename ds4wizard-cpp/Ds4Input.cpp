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
	const Ds4Buttons_t lastDpad = heldButtons & Ds4Buttons::dpad;  // NOLINT(clang-analyzer-deadcode.DeadStores)
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

void Ds4Input::updateAxes(const Ds4InputData& old)
{
	axes = 0;

	if (data.leftStick.x != old.leftStick.x)
	{
		axes |= Ds4Axes::leftStickX;
	}

	if (data.leftStick.y != old.leftStick.y)
	{
		axes |= Ds4Axes::leftStickY;
	}

	if (data.rightStick.x != old.rightStick.x)
	{
		axes |= Ds4Axes::rightStickX;
	}

	if (data.rightStick.y != old.rightStick.y)
	{
		axes |= Ds4Axes::rightStickY;
	}

	if (data.leftTrigger != old.leftTrigger)
	{
		axes |= Ds4Axes::leftTrigger;
	}

	if (data.rightTrigger != old.rightTrigger)
	{
		axes |= Ds4Axes::rightTrigger;
	}

	if (data.accel.x != old.accel.x)
	{
		axes |= Ds4Axes::accelX;
	}
	
	if (data.accel.y != old.accel.y)
	{
		axes |= Ds4Axes::accelY;
	}
	
	if (data.accel.z != old.accel.z)
	{
		axes |= Ds4Axes::accelZ;
	}
	
	if (data.gyro.x != old.gyro.x)
	{
		axes |= Ds4Axes::gyroX;
	}
	
	if (data.gyro.y != old.gyro.y)
	{
		axes |= Ds4Axes::gyroY;
	}
	
	if (data.gyro.z != old.gyro.z)
	{
		axes |= Ds4Axes::gyroZ;
	}
}

void Ds4Input::applySensorCalibration(Ds4Vector3* gyro, Ds4Vector3* accel) const
{
	gyro->x  = static_cast<short>((static_cast<float>(gyro->x) - calibration.gyroPitch.bias) * (calibration.gyroPitch.numerator / calibration.gyroPitch.denominator));
	gyro->y  = static_cast<short>((static_cast<float>(gyro->y) - calibration.gyroYaw.bias) * (calibration.gyroYaw.numerator / calibration.gyroYaw.denominator));
	gyro->z  = static_cast<short>((static_cast<float>(gyro->z) - calibration.gyroRoll.bias) * (calibration.gyroRoll.numerator / calibration.gyroRoll.denominator));
	accel->x = static_cast<short>((static_cast<float>(accel->x) - calibration.accelX.bias) * (calibration.accelX.numerator / calibration.accelX.denominator));
	accel->y = static_cast<short>((static_cast<float>(accel->y) - calibration.accelY.bias) * (calibration.accelY.numerator / calibration.accelY.denominator));
	accel->z = static_cast<short>((static_cast<float>(accel->z) - calibration.accelZ.bias) * (calibration.accelZ.numerator / calibration.accelZ.denominator));
}

void Ds4Input::update(std::span<const uint8_t> buffer)
{
	const Ds4InputData old = data;

	data.leftStick.x       = buffer[0];
	data.leftStick.y       = buffer[1];
	data.rightStick.x      = buffer[2];
	data.rightStick.y      = buffer[3];
	data.frameCount        = static_cast<uint8_t>((buffer[6] >> 2) & 0x3F);
	data.leftTrigger       = buffer[7];
	data.rightTrigger      = buffer[8];
	data.gyro.x            = *reinterpret_cast<const int16_t*>(&buffer[12]);
	data.gyro.y            = *reinterpret_cast<const int16_t*>(&buffer[14]);
	data.gyro.z            = *reinterpret_cast<const int16_t*>(&buffer[16]);
	data.accel.x           = *reinterpret_cast<const int16_t*>(&buffer[18]);
	data.accel.y           = *reinterpret_cast<const int16_t*>(&buffer[20]);
	data.accel.z           = *reinterpret_cast<const int16_t*>(&buffer[22]);
	data.extensions        = static_cast<uint8_t>(buffer[29] >> 4);
	data.battery           = static_cast<uint8_t>(buffer[29] & 0x0F);
	data.touchEvent        = static_cast<uint8_t>(buffer[32] & 0x3F);
	data.touchFrame        = static_cast<uint8_t>(buffer[33] & 0x3F);
	data.touch1            = !(buffer[34] & 0x80);
	data.touch1Id          = static_cast<uint8_t>(buffer[34] & 0x7F);
	data.touchPoint1.x     = static_cast<short>(*reinterpret_cast<const int16_t*>(&buffer[35]) & 0xFFF);
	data.touchPoint1.y     = static_cast<short>((*reinterpret_cast<const int16_t*>(&buffer[36]) >> 4) & 0xFFF);
	data.touch2            = !(buffer[38] & 0x80);
	data.touch2Id          = static_cast<uint8_t>(buffer[38] & 0x7F);
	data.touchPoint2.x     = static_cast<short>(*reinterpret_cast<const int16_t*>(&buffer[39]) & 0xFFF);
	data.touchPoint2.y     = static_cast<short>((*reinterpret_cast<const int16_t*>(&buffer[40]) >> 4) & 0xFFF);
	data.lastTouchPoint1.x = static_cast<short>(*reinterpret_cast<const int16_t*>(&buffer[43]) & 0xFFF);
	data.lastTouchPoint1.y = static_cast<short>((*reinterpret_cast<const int16_t*>(&buffer[44]) >> 4) & 0xFFF);
	data.lastTouchPoint2.x = static_cast<short>(*reinterpret_cast<const int16_t*>(&buffer[47]) & 0xFFF);
	data.lastTouchPoint2.y = static_cast<short>((*reinterpret_cast<const int16_t*>(&buffer[48]) >> 4) & 0xFFF);

	applySensorCalibration(&data.gyro, &data.accel);
	updateAxes(old);

	if (!(data.extensions & Ds4Extensions::cable))
	{
		++data.battery;
	}

	if (data.battery > 10)
	{
		data.battery = 10;
	}

	data.activeButtons = *reinterpret_cast<const Ds4ButtonsRaw_t*>(&buffer[4]);

	updateButtons();
	updateChangedState();
}

void Ds4Input::updateSensorCalibration(std::span<const uint8_t> buffer, bool isUsb)
{
	calibration.gyroPitch.bias = *reinterpret_cast<const int16_t*>(&buffer[1]);
	calibration.gyroYaw.bias   = *reinterpret_cast<const int16_t*>(&buffer[3]);
	calibration.gyroRoll.bias  = *reinterpret_cast<const int16_t*>(&buffer[5]);

	int16_t gyroPitchAdd;
	int16_t gyroPitchSubtract;
	int16_t gyroYawAdd;
	int16_t gyroYawSubtract;
	int16_t gyroRollAdd;
	int16_t gyroRollSubtract;

	if (!isUsb)
	{
		gyroPitchAdd      = *reinterpret_cast<const int16_t*>(&buffer[7]);
		gyroYawAdd        = *reinterpret_cast<const int16_t*>(&buffer[9]);
		gyroRollAdd       = *reinterpret_cast<const int16_t*>(&buffer[11]);
		gyroPitchSubtract = *reinterpret_cast<const int16_t*>(&buffer[13]);
		gyroYawSubtract   = *reinterpret_cast<const int16_t*>(&buffer[15]);
		gyroRollSubtract  = *reinterpret_cast<const int16_t*>(&buffer[17]);
	}
	else
	{
		gyroPitchAdd      = *reinterpret_cast<const int16_t*>(&buffer[7]);
		gyroPitchSubtract = *reinterpret_cast<const int16_t*>(&buffer[9]);
		gyroYawAdd        = *reinterpret_cast<const int16_t*>(&buffer[11]);
		gyroYawSubtract   = *reinterpret_cast<const int16_t*>(&buffer[13]);
		gyroRollAdd       = *reinterpret_cast<const int16_t*>(&buffer[15]);
		gyroRollSubtract  = *reinterpret_cast<const int16_t*>(&buffer[17]);
	}

	const int16_t gyroSpeedAdd      = *reinterpret_cast<const int16_t*>(&buffer[19]);
	const int16_t gyroSpeedSubtract = *reinterpret_cast<const int16_t*>(&buffer[21]);

	const int16_t accelXAdd      = *reinterpret_cast<const int16_t*>(&buffer[23]);
	const int16_t accelXSubtract = *reinterpret_cast<const int16_t*>(&buffer[25]);
	const int16_t accelYAdd      = *reinterpret_cast<const int16_t*>(&buffer[27]);
	const int16_t accelYSubtract = *reinterpret_cast<const int16_t*>(&buffer[29]);
	const int16_t accelZAdd      = *reinterpret_cast<const int16_t*>(&buffer[31]);
	const int16_t accelZSubtract = *reinterpret_cast<const int16_t*>(&buffer[33]);

	const auto gyroSpeed = static_cast<int16_t>(gyroSpeedAdd + gyroSpeedSubtract);
	const auto gyroSpeedNumerator = static_cast<float>(gyroSpeed) * gyroDegreesPerSecond;

	calibration.gyroPitch.numerator   = gyroSpeedNumerator;
	calibration.gyroPitch.denominator = static_cast<float>(gyroPitchAdd - gyroPitchSubtract);

	calibration.gyroYaw.numerator   = gyroSpeedNumerator;
	calibration.gyroYaw.denominator = static_cast<float>(gyroYawAdd - gyroYawSubtract);

	calibration.gyroRoll.numerator   = gyroSpeedNumerator;
	calibration.gyroRoll.denominator = static_cast<float>(gyroRollAdd - gyroRollSubtract);

	// from DS4Windows:
	// Some revision 1 DualShock 4s have an inverted yaw gyroscope axis calibration (where numerator is positive, but the denominator is negative
	if (calibration.gyroYaw.numerator > 0 && calibration.gyroYaw.denominator < 0 &&
	    calibration.gyroPitch.denominator > 0 && calibration.gyroRoll.denominator > 0)
	{
		calibration.gyroYaw.denominator = -calibration.gyroYaw.denominator;
	}

	const auto accelRangeX = static_cast<short>(accelXAdd - accelXSubtract);

	calibration.accelX.bias        = static_cast<float>(accelXAdd) - (static_cast<float>(accelRangeX) / 2.0f);
	calibration.accelX.numerator   = accelResolutionPerG * 2.0f;
	calibration.accelX.denominator = accelRangeX;

	const auto accelRangeY = static_cast<short>(accelYAdd - accelYSubtract);
	
	calibration.accelY.bias        = static_cast<float>(accelYAdd) - (static_cast<float>(accelRangeY) / 2.0f);
	calibration.accelY.numerator   = accelResolutionPerG * 2.0f;
	calibration.accelY.denominator = accelRangeY;

	const auto accelRangeZ = static_cast<short>(accelZAdd - accelZSubtract);
	
	calibration.accelZ.bias        = static_cast<float>(accelZAdd) - (static_cast<float>(accelRangeZ) / 2.0f);
	calibration.accelZ.numerator   = accelResolutionPerG * 2.0f;
	calibration.accelZ.denominator = accelRangeZ;
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

float Ds4Input::getAxis(Ds4Axes_t axis, const std::optional<AxisPolarity>& polarity) const
{
	float result;

	switch (axis)
	{
		case Ds4Axes::leftStickX:
			result = std::clamp(data.leftStick.x - stickAdjust, -stickMax, stickMax) / static_cast<float>(stickMax);
			break;
		case Ds4Axes::leftStickY:
			result = -std::clamp(data.leftStick.y - stickAdjust, -stickMax, stickMax) / static_cast<float>(stickMax);
			break;

		case Ds4Axes::rightStickX:
			result = std::clamp(data.rightStick.x - stickAdjust, -stickMax, stickMax) / static_cast<float>(stickMax);
			break;
		case Ds4Axes::rightStickY:
			result = -std::clamp(data.rightStick.y - stickAdjust, -stickMax, stickMax) / static_cast<float>(stickMax);
			break;

		case Ds4Axes::leftTrigger:
			result = static_cast<float>(data.leftTrigger) / triggerMax;
			break;
		case Ds4Axes::rightTrigger:
			result = static_cast<float>(data.rightTrigger) / triggerMax;
			break;

		case Ds4Axes::accelX:
			result = std::clamp<int>(data.accel.x, accelMin, accelMax) / accelResolutionPerG;
			break;
		case Ds4Axes::accelY:
			result = -std::clamp<int>(data.accel.y, accelMin, accelMax) / accelResolutionPerG;
			break;
		case Ds4Axes::accelZ:
			result = std::clamp<int>(data.accel.z, accelMin, accelMax) / accelResolutionPerG;
			break;

		case Ds4Axes::gyroX:
			result = -std::clamp<int>(data.gyro.x, gyroMin, gyroMax) / gyroDegreesPerSecond;
			break;
		case Ds4Axes::gyroY:
			result = std::clamp<int>(data.gyro.y, gyroMin, gyroMax) / gyroDegreesPerSecond;
			break;
		case Ds4Axes::gyroZ:
			result = std::clamp<int>(data.gyro.z, gyroMin, gyroMax) / gyroDegreesPerSecond;
			break;

		default:
			throw std::out_of_range("invalid Ds4Axes");
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
