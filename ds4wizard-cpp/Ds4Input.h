#pragma once

#include <memory>
#include <gsl/span>
#include "Ds4InputData.h"
#include "ScpDevice.h"
#include "XInputGamepad.h"
#include <optional>

class Ds4Input
{
public:
	Ds4Input() = default;
	
	Ds4Buttons_t heldButtons = 0; // TODO: private set
	Ds4Buttons_t pressedButtons = 0; // TODO: private set
	Ds4Buttons_t releasedButtons = 0; // TODO: private set

private:
	Ds4Buttons_t lastHeldButtons = 0;

public:
	bool buttonsChanged = false; // TODO: private set
	bool touchChanged = false; // TODO: private set

	Ds4InputData data {};
	XInputGamepad gamepad {};

private:
	XInputGamepad lastGamepad {};
	uint8_t lastTouchFrame {};

	void addButton(bool pressed, Ds4Buttons_t buttons);

public:
	void update(const gsl::span<uint8_t>& buffer);
	void updateChangedState();
	void toXInput(int index, std::unique_ptr<ScpDevice>& device);
	float getAxis(Ds4Axis_t axis, const std::optional<AxisPolarity>& polarity) const;
};
