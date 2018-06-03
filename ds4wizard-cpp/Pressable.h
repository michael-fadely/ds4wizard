#pragma once

#include "enums.h"

class Pressable
{
public:
	PressedState State = PressedState::Off;

	virtual bool IsActive() const;
	static void Press(PressedState& state);
	static void Release(PressedState& state);
	static bool IsActiveState(PressedState state);
	virtual void Press();
	virtual void Release();
};