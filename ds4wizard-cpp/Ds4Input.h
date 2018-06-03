#pragma once

#include <memory>
#include <gsl/span>
#include "Ds4Buttons.h"
#include "Ds4InputData.h"
#include "ScpDevice.h"
#include "XInputGamepad.h"

class Ds4Input
{
public:
	Ds4Buttons_t HeldButtons = 0; // TODO: private set
	Ds4Buttons_t PressedButtons = 0; // TODO: private set
	Ds4Buttons_t ReleasedButtons = 0; // TODO: private set

private:
	Ds4Buttons_t LastHeldButtons = 0;

public:
	bool ButtonsChanged = false; // TODO: private set
	bool TouchChanged = false; // TODO: private set

	Ds4InputData Data {};
	XInputGamepad Gamepad {};

private:
	XInputGamepad lastGamepad {};
	uint8_t lastTouchFrame {};

	void _addButton(uint8_t pressed, Ds4Buttons_t buttons);

public:
	void Update(const gsl::span<uint8_t>& buffer, int i);
	void UpdateChangedState();
	void ToXInput(int index, std::unique_ptr<ScpDevice>& device);
	float GetAxis(Ds4Axis_t axis, AxisPolarity* polarity) const;
};
