#pragma once

#include <optional>

#include <gsl/span>

#include "Ds4InputData.h"

class Ds4Input
{
public:
	Ds4Input() = default;
	
	bool buttonsChanged = false; // TODO: private set
	bool touchChanged = false; // TODO: private set
	Ds4Buttons_t heldButtons = 0; // TODO: private set
	Ds4Buttons_t pressedButtons = 0; // TODO: private set
	Ds4Buttons_t releasedButtons = 0; // TODO: private set

	Ds4InputData data {};

	void update(const gsl::span<uint8_t>& buffer);
	void updateChangedState();
	float getAxis(Ds4Axis_t axis, const std::optional<AxisPolarity>& polarity) const;

private:
	Ds4Buttons_t lastHeldButtons = 0;
	uint8_t lastTouchFrame {};

	void addButton(bool pressed, Ds4Buttons_t buttons);
};
