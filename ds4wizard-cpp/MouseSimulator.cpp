#include "pch.h"
#include <Windows.h>
#include "MouseSimulator.h"

MouseSimulator::~MouseSimulator()
{
	for (auto button : pressedButtons)
	{
		buttonUp(MouseButton::_from_integral(button));
	}
}

void MouseSimulator::buttonUp(MouseButton button)
{
	pressedButtons.insert(button);
	press(button, false);
}

void MouseSimulator::buttonDown(MouseButton button)
{
	pressedButtons.erase(button);
	press(button, true);
}

void MouseSimulator::moveBy(int dx, int dy)
{
	INPUT input;

	input.type       = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_MOVE;
	input.mi.dx      = dx;
	input.mi.dy      = dy;

	SendInput(1, &input, sizeof(INPUT));
}

void MouseSimulator::press(MouseButton button, bool down)
{
	INPUT input;

	input.type = INPUT_MOUSE;

	switch (button)
	{
		case MouseButton::left:
			input.mi.dwFlags = (down ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP);
			break;

		case MouseButton::right:
			input.mi.dwFlags = (down ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP);
			break;

		case MouseButton::middle:
			input.mi.dwFlags = (down ? MOUSEEVENTF_MIDDLEDOWN : MOUSEEVENTF_MIDDLEUP);
			break;

		case MouseButton::ex1:
			input.mi.dwFlags = (down ? MOUSEEVENTF_XDOWN : MOUSEEVENTF_XUP);
			input.mi.mouseData = XBUTTON1;
			break;

		case MouseButton::ex2:
			input.mi.dwFlags = (down ? MOUSEEVENTF_XDOWN : MOUSEEVENTF_XUP);
			input.mi.mouseData = XBUTTON2;
			break;

		default:
			return;
	}

	SendInput(1, &input, sizeof(INPUT));
}
