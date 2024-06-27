#pragma once

#include <optional>
#include <span>

#include "Ds4InputData.h"

struct Ds4SensorCalibrationData
{
	float bias;
	float numerator;
	float denominator;
};

struct Ds4SensorCalibration
{
	Ds4SensorCalibrationData gyroPitch;
	Ds4SensorCalibrationData gyroYaw;
	Ds4SensorCalibrationData gyroRoll;

	Ds4SensorCalibrationData accelX;
	Ds4SensorCalibrationData accelY;
	Ds4SensorCalibrationData accelZ;
};

/**
 * \brief Serialized input report from a \c Ds4Device
 * \sa Ds4Device
 */
class Ds4Input
{
public:
	Ds4Input() = default;
	
	/**
	 * \brief Indicates if button press state has changed since the last poll.
	 */
	bool buttonsChanged = false; // TODO: private set

	/**
	 * \brief Indicates if touch state has changed since last poll.
	 */
	bool touchChanged = false; // TODO: private set

	/**
	 * \brief Buttons currently held.
	 * \sa Ds4Buttons, Ds4Buttons_t
	 */
	Ds4Buttons_t heldButtons = 0; // TODO: private set

	/**
	 * \brief Buttons pressed since last poll.
	 * \sa Ds4Buttons, Ds4Buttons_t
	 */
	Ds4Buttons_t pressedButtons = 0; // TODO: private set

	/**
	 * \brief Buttons released since last poll.
	 * \sa Ds4Buttons, Ds4Buttons_t
	 */
	Ds4Buttons_t releasedButtons = 0; // TODO: private set

	/**
	 * \brief Each axis that has changed since the last poll.
	 * \sa Ds4Axes, Ds4Axes_t
	 */
	Ds4Axes_t axes = 0; // TODO: private set

	Ds4InputData data {};

	static constexpr int stickMin = -128;
	static constexpr int stickMax = 127;
	static constexpr int stickAdjust = -stickMin; // used to shift the range from [0, 255] to [-127, 127]
	static constexpr int triggerMax = 255;

	// See BOSCH BMI055 spec sheet:
	// https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bmi055-ds000.pdf
	static constexpr int accelMin = -32767;
	static constexpr int accelMax = 32767;
	static constexpr int gyroMin = -32767;
	static constexpr int gyroMax = 32767;

	static constexpr float gyroDegreesPerSecond = gyroMax / 2000.0f;
	static constexpr float accelResolutionPerG = accelMax / 4.0f;

	/**
	 * \brief Updates serialized data using the given buffer.
	 * \param buffer Buffer containing raw input report data.
	 */
	void update(std::span<const uint8_t> buffer);

	/**
	 * \brief Updates gyroscope and accelerometer calibration the data from the given buffer.
	 * \param buffer Buffer containing raw calibration data.
	 * \param isUsb \p buffer was read from the USB interface. Data is arranged differently in this scenario.
	 */
	void updateSensorCalibration(std::span<const uint8_t> buffer, bool isUsb);

	/**
	 * \brief Updates button change states since last poll.
	 */
	void updateChangedState();

	/**
	 * \brief Get the magnitude of an axis.
	 * \param axis The axis to retrieve.
	 * \param polarity The desired polarity of the axis, or \c std::nullopt for both positive and negative.
	 * \return The magnitude of the axis. If it does not align with the desired \a polarity, \c 0.0f is returned.
	 */
	[[nodiscard]] float getAxis(Ds4Axes_t axis, const std::optional<AxisPolarity>& polarity) const;

private:
	Ds4SensorCalibration calibration {};

	Ds4Buttons_t lastHeldButtons = 0;
	uint8_t lastTouchFrame {};

	void addButton(bool pressed, Ds4Buttons_t buttons);
	void updateButtons();
	void updateAxes(const Ds4InputData& old);
	void applySensorCalibration(Ds4Vector3* gyro, Ds4Vector3* accel) const;
};
