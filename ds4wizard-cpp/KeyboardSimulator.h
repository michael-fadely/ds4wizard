#pragma once

#include <unordered_set>

class KeyboardSimulator
{
	std::unordered_set<int> pressedKeys;

public:
	KeyboardSimulator() = default;
	KeyboardSimulator(KeyboardSimulator&&) = default;

	~KeyboardSimulator();

	KeyboardSimulator(const KeyboardSimulator&) = delete;
	KeyboardSimulator& operator=(KeyboardSimulator&&) = default;
	KeyboardSimulator& operator=(const KeyboardSimulator&) = delete;

	void keyUp(int keyCode);
	void keyDown(int keyCode);

	static void press(int keyCode, bool down);
};
