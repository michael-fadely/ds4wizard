#include "stdafx.h"
#include "Pressable.h"

bool Pressable::IsActive() const
{
	return IsActiveState(State);
}

void Pressable::Press(PressedState& state)
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

void Pressable::Release(PressedState& state)
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

bool Pressable::IsActiveState(PressedState state)
{
	return state == PressedState::On || state == PressedState::Pressed;
}

void Pressable::Press()
{
	Press(State);
}

void Pressable::Release()
{
	Release(State);
}
