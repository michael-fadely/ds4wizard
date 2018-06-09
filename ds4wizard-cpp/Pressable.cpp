#include "stdafx.h"
#include "Pressable.h"

bool Pressable::isActive() const
{
	return isActiveState(pressedState);
}

void Pressable::press(PressedState& state)
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

void Pressable::release(PressedState& state)
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

bool Pressable::isActiveState(PressedState state)
{
	return state == PressedState::on || state == PressedState::pressed;
}

void Pressable::press()
{
	press(pressedState);
}

void Pressable::release()
{
	release(pressedState);
}
