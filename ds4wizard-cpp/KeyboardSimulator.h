#pragma once

#include <unordered_set>

/**
 * \brief Object used for simulating keyboard input.
 */
class KeyboardSimulator
{
	std::unordered_set<int> pressedKeys;

public:
	KeyboardSimulator() = default;
	KeyboardSimulator(KeyboardSimulator&&) = default;

	~KeyboardSimulator();

	KeyboardSimulator& operator=(KeyboardSimulator&&) = default;

	/**
	 * \brief Explicitly disallow copying.
	 */
	KeyboardSimulator(const KeyboardSimulator&) = delete;

	/**
	 * \brief Explicitly disallow copying.
	 */
	KeyboardSimulator& operator=(const KeyboardSimulator&) = delete;

	/**
	 * \brief Releases a key specified by \a keyCode.
	 * \param keyCode The key code to release.
	 */
	void keyUp(int keyCode);

	/**
	 * \brief Presses a key specified by \a keyCode.
	 * \param keyCode The key code to press.
	 */
	void keyDown(int keyCode);

	/**
	 * \brief Presses or releases a key specified by \a keyCode.
	 * \param keyCode The key code to press or release.
	 * \param down If \c true, the key is pressed. If \c false, the key is released.
	 */
	static void press(int keyCode, bool down);
};
