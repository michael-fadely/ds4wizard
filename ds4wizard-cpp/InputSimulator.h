#pragma once

#include <functional>
#include <unordered_set>
#include <unordered_map>

#include "KeyboardSimulator.h"
#include "MouseSimulator.h"
#include "enums.h"
#include "Pressable.h"
#include "InputMap.h"
#include "XInputGamepad.h"
#include "ViGEmTarget.h"
#include "MapCache.h"
#include "ISimulator.h"
#include "XInputRumbleSimulator.h"
#include "RumbleSequence.h"

class Ds4Device;

/**
 * \brief Class for handling input maps and outputting simulated inputs to simulated input devices (keyboard, mouse, XInput, etc)
 */
class InputSimulator
{
	static constexpr Ds4Buttons_t touchMask = Ds4Buttons::touch1 | Ds4Buttons::touch2;

	Ds4Device* parent = nullptr;

	KeyboardSimulator keyboard;
	MouseSimulator mouse;

	Ds4TouchRegionCache touchRegions;
	std::vector<Ds4TouchRegion*> sortableTouchRegions;

	std::unordered_map<InputModifier*, MapCacheCollection<InputMap>> modifierMaps;
	MapCacheCollection<InputModifier> modifiers;
	MapCacheCollection<InputMap> bindings;

	std::unordered_set<InputMap*> activeMaps;
	std::unordered_set<InputModifier*> activeModifiers;

	int realXInputIndex = -1;
	XInputGamepad xinputPad {};
	XInputGamepad xinputLast {};
	std::shared_ptr<vigem::XInputTarget> xinputTarget;
	XInputAxis_t simulatedXInputAxis = 0;

	// Delta time (for things like mouse movement).
	// Assumes 1000 Hz virtual polling rate.
	Stopwatch deltaStopwatch {};

	// Delta time (for things like mouse movement).
	// Assumes 1000 Hz virtual polling rate.
	float deltaTime = 1.0f;

	std::unique_ptr<XInputRumbleSimulator> xinputRumbleSimulator;
	std::unordered_set<ISimulator*> simulators;
	std::unique_ptr<RumbleSequence> rumbleSequence;

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

	void start();

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
	 * \brief Checks if the given input map is overridden by an input map from the currently-active modifier set.
	 * \param map The map whose overridden state is to be checked.
	 * \return \c true if overridden by a modifier.
	 */
	bool isOverriddenByModifierSet(InputMapBase& map);

	/**
	 * \brief Runs an input map with an optional parent modifier.
	 * \param m The map to run.
	 * \param modifier The parent modifier, if any.
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

	void updateDeltaTime();
	void updateModifiers();
	void updateBindings();
	void reset();

	void setRumble(uint8_t leftMotor, uint8_t rightMotor);

	bool addSimulator(ISimulator* simulator);
	bool removeSimulator(ISimulator* simulator);
	void runSimulators();

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
	 * \param sender The touch sender (touch A or touch B)
	 * \param point The point on the touch pad that \a sender was fired from.
	 * \param disallow Buttons to disallow if a region does not allow overlap.
	 */
	void updateTouchRegion(Ds4TouchRegion& region, Ds4Buttons_t sender,
	                       Ds4Vector2& point, Ds4Buttons_t& disallow) const;
	
	/**
	 * \brief Internal implementation of \sa updatePressedState
	 * \param instance Input map whose state is to be updated.
	 * \param press Press callback.
	 * \param release Release callback.
	 */
	void updatePressedStateImpl(InputMapBase& instance, const std::function<void()>& press, const std::function<void()>& release);
	void updateActiveModifiers(InputModifier& modifier);

	/**
	 * \brief Updates the pressed state of a modifier set and its managed child bindings.
	 * \param modifier The modifier to update.
	 * \return \c true if the active state of the modifier has changed.
	 */
	bool updateModifier(InputModifier& modifier);
	
	/**
	 * \brief
	 * Generic method for updating the pressed state of a binding.
	 * If provided, the parent \p modifier must be active for \p map to be pressed.
	 * Otherwise, the map's pressed state is made inactive.
	 * 
	 * \param map The map to update.
	 * \param modifier The parent modifier set, if any.
	 */
	bool updatePressedState(InputMap& map, InputModifier* modifier);
	void updateActiveMaps(InputMap& map);

	/**
	 * \brief
	 * Root level method for updating input mappings and simulating inputs.
	 * Updates pressed state accounting for modifiers, runs special actions, etc.
	 * If provided, the parent \p modifier must be active for \p map to be activated.
	 * Otherwise, the map's pressed state is made inactive.
	 * 
	 * \param map The map to update.
	 * \param modifier The parent modifier set, if any.
	 *
	 * \return \c true if the pressed state of the map has changed.
	 */
	bool updateBindingState(InputMap& map, InputModifier* modifier);
	
private:
	/**
	 * \brief Connects a virtual XInput device to the system.
	 * \return \c true on success.
	 */
	bool xinputConnect();

	/**
	 * \brief Disconnects a virtual XInput device from the system.
	 */
	void xinputDisconnect();

	/**
	 * \brief Acquires a handle to the XInput emulation driver.
	 * \return \c true on success.
	 */
	bool xinputDeviceOpen();

	/**
	 * \brief Closes the handle to the XInput emulation driver.
	 */
	void xinputDeviceClose();
};
