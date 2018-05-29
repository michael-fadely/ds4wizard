#pragma once

#include "enums.h"

class Pressable
{
public:
	/*[JsonIgnore]*/ PressedState State = PressedState::Off;

	/*[JsonIgnore]*/
	virtual bool IsActive() const { return IsActiveState(State); }

	static void Press(PressedState& state)
	{
		switch (state)
		{
			case PressedState::Off:
			case PressedState::Released:
				state = PressedState::Pressed;
				break;

			default:
				state = PressedState::On;
				break;
		}
	}

	static void Release(PressedState& state)
	{
		switch (state)
		{
			case PressedState::On:
			case PressedState::Pressed:
				state = PressedState::Released;
				break;

			default:
				state = PressedState::Off;
				break;
		}
	}

	static bool IsActiveState(PressedState state)
	{
		return state == PressedState::On || state == PressedState::Pressed;
	}

	virtual void Press()
	{
		Press(State);
	}

	virtual void Release()
	{
		Release(State);
	}
};