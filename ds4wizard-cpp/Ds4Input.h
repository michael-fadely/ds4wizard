#pragma once

#include "Ds4Buttons.h"
#include "Ds4InputData.h"
#include <gsl/span>
#include "ScpDevice.h"
#include "XInputGamepad.h"

class Ds4Input
{
public:
	Ds4Buttons::T HeldButtons; // TODO: private set
	Ds4Buttons::T PressedButtons; // TODO: private set
	Ds4Buttons::T ReleasedButtons; // TODO: private set

private:
	Ds4Buttons::T LastHeldButtons;

public:
	bool ButtonsChanged = false; // TODO: private set
	bool TouchChanged = false; // TODO: private set

	Ds4InputData Data {};
	XInputGamepad Gamepad {};

private:
	XInputGamepad lastGamepad {};
	uint8_t lastTouchFrame {};

	void _addButton(uint8_t pressed, Ds4Buttons::T buttons);

public:
	void Update(gsl::span<uint8_t> buffer, int i);
	void UpdateChangedState();
	void ToXInput(int index, ScpDevice device);
	float GetAxis(Ds4Axis axis, AxisPolarity* polarity);
};
