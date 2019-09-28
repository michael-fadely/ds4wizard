#pragma once

#include <unordered_set>
#include "enums.h"

/**
 * \brief An object for simulating mouse input.
 */
class MouseSimulator
{
	std::unordered_set<MouseButton::_integral> pressedButtons;

public:
	MouseSimulator() = default;
	MouseSimulator(MouseSimulator&&) = default;

	~MouseSimulator();

	/**
	 * \brief Explicitly disallow copying.
	 */
	MouseSimulator(const MouseSimulator&) = delete;

	/**
	 * \brief Explicitly disallow copying.
	 */
	MouseSimulator& operator=(const MouseSimulator&) = delete;

	MouseSimulator& operator=(MouseSimulator&&) = default;

	/**
	 * \brief Releases a mouse button.
	 * \param button The button to release.
	 */
	void buttonUp(MouseButton button);

	/**
	 * \brief Presses a mouse button.
	 * \param button The button to press.
	 */
	void buttonDown(MouseButton button);

	/**
	 * \brief Moves the cursor relative to its current position.
	 * \param dx X delta to move by, in pixels.
	 * \param dy Y delta to move by, in pixels.
	 */
	static void moveBy(int dx, int dy);

	/**
	 * \brief Presses or releases a mouse button.
	 * \param button The button to press or release.
	 * \param down If \c true, the button is pressed. If \c false, the button is released.
	 */
	static void press(MouseButton button, bool down);
};
