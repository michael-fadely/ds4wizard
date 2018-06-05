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
		case PressedState::off:
		case PressedState::released:
			state = PressedState::pressed;
			break;

		default:
			state = PressedState::on;
			break;
	}
}

void Pressable::Release(PressedState& state)
{
	switch (state)
	{
		case PressedState::on:
		case PressedState::pressed:
			state = PressedState::released;
			break;

		default:
			state = PressedState::off;
			break;
	}
}

bool Pressable::IsActiveState(PressedState state)
{
	return state == PressedState::on || state == PressedState::pressed;
}

void Pressable::Press()
{
	Press(State);
}

void Pressable::Release()
{
	Release(State);
}
