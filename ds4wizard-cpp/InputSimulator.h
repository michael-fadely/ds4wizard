#pragma once

#include <deque>
#include <functional>

#include "KeyboardSimulator.h"
#include "MouseSimulator.h"
#include "enums.h"
#include "Pressable.h"
#include "InputMap.h"
#include "XInputGamepad.h"
#include "ViGEmTarget.h"

class Ds4Device;

/**
 * \brief Class for handling input maps and outputting simulated inputs to simulated input devices (keyboard, mouse, XInput, etc)
 */
class InputSimulator
{
	static constexpr Ds4Buttons_t touchMask = Ds4Buttons::touch1 | Ds4Buttons::touch2;

	Ds4Device* parent = nullptr;
	DeviceProfile* profile = nullptr;

	KeyboardSimulator keyboard;
	MouseSimulator mouse;

	/** \brief Bindings affected by modifier sets. */
	std::deque<InputMap*> modifier_bindings; // TODO: OPTIMIZE !!!
	/** \brief Cache for touch regions. */
	std::deque<Ds4TouchRegion*> touchRegions;

	int realXInputIndex = -1;
	XInputGamepad xinputPad {};
	XInputGamepad xinputLast {};
	XINPUT_VIBRATION xinputVibration {};
	std::unique_ptr<vigem::XInputTarget> xinputTarget;
	XInputAxis_t simulatedXInputAxis = 0;
	EventToken xinputNotification;

public:
	/** \brief Event raised when a handle to a virtual XInput device cannot be acquired. */
	Event<Ds4Device> onXInputHandleFailure;

	/** \brief \c InputSimulator cannot be copied or moved. */
	InputSimulator() = delete;
	/** \brief \c InputSimulator cannot be copied or moved. */
	InputSimulator(const InputSimulator&) = delete;
	/** \brief \c InputSimulator cannot be copied or moved. */
	InputSimulator(InputSimulator&&) = delete;
	/** \brief \c InputSimulator cannot be copied or moved. */
	InputSimulator& operator=(const InputSimulator&) = delete;
	/** \brief \c InputSimulator cannot be copied or moved. */
	InputSimulator& operator=(InputSimulator&&) = delete;

	explicit InputSimulator(Ds4Device* parent);
	~InputSimulator();

	/**
	 * \brief Simulates XInput buttons.
	 * \param buttons Buttons to simulate.
	 * \param state The state of \a buttons
	 */
	void simulateXInputButton(XInputButtons_t buttons, PressedState state);

	/**
	 * \brief Simulates XInput axes.
	 * \param axes The axes to simulate.
	 * \param m The magnitude of the axes.
	 */
	void simulateXInputAxis(XInputAxes& axes, float m);

	/**
	 * \brief Caches \c InputMap instances which are controlled by an \c InputModifier.
	 * Excludes the provided map under the assumption that it is not handled by any \c InputModifier.
	 * This information will be used later in \c isOverriddenByModifierSet
	 * \param map The input map to be excluded.
	 */
	void cacheModifierBindings(InputMapBase& map);

	/**
	 * \brief Checks if an input map is overriden by a currently active modifier set.
	 * \param map The map whose overridden state is to be checked.
	 * \return \c true if overridden by a modifier.
	 */
	bool isOverriddenByModifierSet(InputMapBase& map);

	/**
	 * \brief Runs an input map with an optional parent modifier set.
	 * \param m The map to run.
	 * \param modifier The parent modifier set, if any.
	 */
	void runMap(InputMap& m, InputModifier* modifier);

	/**
	 * \brief Applies a profile to the device.
	 * \param profile The profile to apply.
	 */
	void applyProfile(DeviceProfile* profile);

	/**
	 * \brief Handles toggles which are managed by touch regions.
	 * \param m The input map managed by a touch region.
	 * \param modifier Parent modifier set, if any.
	 * \param pressable The pressable state of the touch region.
	 * \return The simulated pressed state.
	 */
	static PressedState handleTouchToggle(InputMap& m, InputModifier* modifier, const Pressable& pressable);

	/**
	 * \brief Applies a map's state as determined by \sa runMap
	 * \param m The mapping to apply.
	 * \param modifier Parent modifier set, if any.
	 * \param state The pressed state to apply, if applicable.
	 * \param analog Analog value to apply, if applicable.
	 */
	void applyMap(InputMap& m, InputModifier* modifier, PressedState state, float analog);

	/**
	 * \brief Simulates mouse inputs.
	 * \param m Map state information to simulate as mouse input.
	 * \param state Pressed state, if applicable.
	 * \param analog Analog value, if applicable.
	 */
	void simulateMouse(const InputMap& m, PressedState state, float analog);
	
	/**
	 * \brief Simulates keyboard key presses.
	 * \param m Map state information to simulate as keyboard input.
	 * \param state Pressed state to apply.
	 */
	void simulateKeyboard(const InputMap& m, PressedState state);
	
	/**
	 * \brief Performs a special action, such as powering off wireless devices.
	 * \param action The action to perform.
	 */
	void runAction(ActionType action) const;
	
	/**
	 * \brief Runs all input maps managed by this instance.
	 */
	void runMaps();
	
	/**
	 * \brief Runs all persistent input maps (e.g rapid fire) managed by this instance.
	 */
	void runPersistent();
	
	/**
	 * \brief Runs all touch regions managed by this instance.
	 * \sa updateTouchRegion
	 */
	void updateTouchRegions();

	/**
	 * \brief Updates a provided touch region, handling allowed/disallowed overlap, etc.
	 * \param region The touch region to update.
	 * \param modifier The parent modifier set, if any.
	 * \param sender The touch sender (touch A or touch B)
	 * \param point The point on the touch pad that \a sender was fired from.
	 * \param disallow Buttons to disallow if a region does not allow overlap.
	 */
	void updateTouchRegion(Ds4TouchRegion& region, InputModifier* modifier, Ds4Buttons_t sender,
	                       Ds4Vector2& point, Ds4Buttons_t& disallow);
	
	/**
	 * \brief Internal implementation of \sa updatePressedState
	 * \param instance Input map whose state is to be updated.
	 * \param press Press callback.
	 * \param release Release callback.
	 */
	void updatePressedStateImpl(InputMapBase& instance, const std::function<void()>& press, const std::function<void()>& release);
	
	/**
	 * \brief Updates the pressed state of a modifier set and its managed child bindings.
	 * \param modifier The modifier to update.
	 */
	void updatePressedState(InputModifier& modifier);
	
	/**
	 * \brief Generic method for updating the pressed state of a binding.
	 * If provided, the parent modifier set must be active for a press to activate.
	 * Otherwise, the map's pressed state is made inactive.
	 * \param map The map to update.
	 * \param modifier The parent modifier set, if any.
	 */
	void updatePressedState(InputMap& map, InputModifier* modifier);
	
	/**
	 * \brief Root level method for updating input mappings and simulating inputs.
	 * Updates pressed state accounting for modifiers, runs special actions, etc.
	 * If provided, the parent modifier set must be active for a press to activate.
	 * Otherwise, the map's pressed state is made inactive.
	 * \param m The map to update.
	 * \param modifier The parent modifier set, if any.
	 */
	void updateBindingState(InputMap& m, InputModifier* modifier);
	
	/**
	 * \brief Polls information from emulation endpoints (e.g XInput) if necessary.
	 */
	void updateEmulators() const;

private:
	/**
	 * \brief Connects a virtual XInput device to the system.
	 * \return \c true on success.
	 */
	bool scpConnect(); // TODO: rename

	/**
	 * \brief Disconnects a virtual XInput device from the system.
	 */
	void scpDisconnect(); // TODO: rename

	/**
	 * \brief Acquires a handle to the XInput emulation driver.
	 * \return \c true on success.
	 */
	bool scpDeviceOpen(); // TODO: rename

	/**
	 * \brief Closes the handle to the XInput emulation driver.
	 */
	void scpDeviceClose(); // TODO: rename
};
