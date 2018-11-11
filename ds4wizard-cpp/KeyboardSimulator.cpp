#include "stdafx.h"
#include <Windows.h>
#include "KeyboardSimulator.h"

KeyboardSimulator::~KeyboardSimulator()
{
	for (auto keyCode : pressedKeys)
	{
		keyUp(keyCode);
	}
}

void KeyboardSimulator::keyUp(int keyCode)
{
	pressedKeys.erase(keyCode);
	press(keyCode, false);
}

void KeyboardSimulator::keyDown(int keyCode)
{
	pressedKeys.insert(keyCode);
	press(keyCode, true);
}

void KeyboardSimulator::press(int keyCode, bool down)
{
	INPUT input;

	input.type       = INPUT_KEYBOARD;
	input.ki.wScan   = static_cast<WORD>(MapVirtualKey(keyCode, MAPVK_VK_TO_VSC));
	input.ki.dwFlags = KEYEVENTF_SCANCODE;

	if ((keyCode > 32 && keyCode < 47) || (keyCode > 90 && keyCode < 94))
	{
		input.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
	}

	if (!down)
	{
		input.ki.dwFlags |= KEYEVENTF_KEYUP; // 0 indicates pressed
	}

	SendInput(1, &input, sizeof(INPUT));
}
