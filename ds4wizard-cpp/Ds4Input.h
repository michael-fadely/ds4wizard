#pragma once

#include <optional>

#include <gsl/span>

#include "Ds4InputData.h"

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

	Ds4InputData data {};

	/**
	 * \brief Updates serialized data using the given buffer.
	 * \param buffer Buffer containing raw input report data.
	 */
	void update(const gsl::span<uint8_t>& buffer);

	/**
	 * \brief Updates button change states since last poll.
	 */
	void updateChangedState();

	/**
	 * \brief Get the magnitude of an axis.
	 * \param axis The axis to retrieve.
	 * \param polarity The desired polarity of the axis.
	 * \return The magnitude of the axis. If it does not align with the desired \a polarity, \c 0.0f is returned.
	 */
	float getAxis(Ds4Axis_t axis, const std::optional<AxisPolarity>& polarity) const;

private:
	Ds4Buttons_t lastHeldButtons = 0;
	uint8_t lastTouchFrame {};

	void addButton(bool pressed, Ds4Buttons_t buttons);
	void updateButtons();
};
