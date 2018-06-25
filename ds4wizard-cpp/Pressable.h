#pragma once

#include "enums.h"

class Pressable
{
public:
	PressedState pressedState = PressedState::off;

	virtual bool isActive() const;
	static void press(PressedState& state);
	static void release(PressedState& state);
	static bool isActiveState(PressedState state);
	virtual void press();
	virtual void release();
};
