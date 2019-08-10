#pragma once

#include "enums.h"

/**
 * \brief An object representing a digital button's pressed state.
 * \sa PressedState
 */
class Pressable
{
public:
	/**
	 * \brief Indicates the pressed state of this pressable instance.
	 * \sa PressedState
	 */
	PressedState pressedState = PressedState::off;

	/**
	 * \brief Indicates that this instance is in an active state, that is: just pressed or held.
	 * \return \c true if active.
	 */
	[[nodiscard]] virtual bool isActive() const;

	/**
	 * \brief Changes a \c PressedState from an inactive to active state.
	 * If \c PressedState::off or \c PressedState::released, the state will be changed to \c PressedState::pressed.
	 * If \c PressedState::pressed, the state is changed to \c PressedState::on
	 * \param state The state to modify.
	 * \sa PressedState
	 */
	static void press(PressedState& state);

	/**
	 * \brief Changes a \c PressedState from an active to inactive state.
	 * If \c PressedState::on or \c PressedState::pressed, the state will be changed to \c PressedState::released.
	 * If \c PressedState::released, the state is changed to \c PressedState::off
	 * \param state The state to modify.
	 * \sa PressedState
	 */
	static void release(PressedState& state);

	/**
	 * \brief Indicates that this instance is in an active state, that is: just pressed or held.
	 * \param state The state to check.
	 * \return \c true if active.
	 * \sa PressedState
	 */
	static bool isActiveState(PressedState state);

	/**
	 * \brief Changes the state of this instance from an inactive to active state.
	 * If \c PressedState::off or \c PressedState::released, the state will be changed to \c PressedState::pressed.
	 * If \c PressedState::pressed, the state is changed to \c PressedState::on
	 * \sa PressedState
	 */
	virtual void press();

	/**
	 * \brief Changes the state of this instance from an active to inactive state.
	 * If \c PressedState::on or \c PressedState::pressed, the state will be changed to \c PressedState::released.
	 * If \c PressedState::released, the state is changed to \c PressedState::off
	 * \sa PressedState
	 */
	virtual void release();
};
