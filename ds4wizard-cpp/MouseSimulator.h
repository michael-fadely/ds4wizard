#pragma once

#include <unordered_set>
#include "enums.h"

class MouseSimulator
{
	std::unordered_set<MouseButton::_integral> pressedButtons;

public:
	MouseSimulator() = default;
	MouseSimulator(MouseSimulator&&) = default;

	~MouseSimulator();

	MouseSimulator(const MouseSimulator&) = delete;
	MouseSimulator& operator=(MouseSimulator&&) = default;
	MouseSimulator& operator=(const MouseSimulator&) = delete;

	void buttonUp(MouseButton button);
	void buttonDown(MouseButton button);

	static void moveBy(int dx, int dy);

	static void press(MouseButton button, bool down);
};
