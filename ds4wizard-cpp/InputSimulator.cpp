#include "pch.h"

#include <chrono>
#include <unordered_set>

#include "Ds4Device.h"
#include "InputSimulator.h"
#include "XInputRumbleSimulator.h"
#include "RumbleSequence.h"

using namespace std::chrono;

InputSimulator::InputSimulator(Ds4Device* parent)
	: parent(parent)
{
	xinputDeviceOpen();
}

InputSimulator::~InputSimulator()
{
	xinputDisconnect();
}

void InputSimulator::start()
{
	deltaStopwatch.start();
}

void InputSimulator::simulateXInputButton(XInputButtons_t buttons, PressedState state)
{
	XInputButtons_t dest = xinputPad.wButtons;

	switch (state)
	{
		case PressedState::off:
			break;

		case PressedState::released:
			dest &= ~buttons;
			break;

		case PressedState::on:
		case PressedState::pressed:
			dest |= buttons;
			break;

		default:
			throw std::out_of_range("invalid PressedState");
	}

	xinputPad.wButtons = dest;
}

void InputSimulator::simulateXInputAxis(const XInputAxes& axes, float m)
{
	for (XInputAxis_t bit : XInputAxis_values)
	{
		if (!(axes.axes & bit))
		{
			continue;
		}

		AxisOptions options = axes.getAxisOptions(static_cast<XInputAxis::T>(bit));

		const auto trigger = static_cast<uint8_t>(255.0f * m);

		const auto axis = static_cast<short>(std::numeric_limits<short>::max() * m);

		const short workAxis = options.polarity == +AxisPolarity::negative
		                       ? static_cast<short>(-axis)
		                       : axis;

		const bool isFirst = !(simulatedXInputAxis & bit);
		simulatedXInputAxis |= bit;

		switch (bit)
		{
			case XInputAxis::leftStickX:
				if (isFirst || axis > std::abs(xinputPad.sThumbLX))
				{
					xinputPad.sThumbLX = workAxis;
				}

				break;

			case XInputAxis::leftStickY:
				if (isFirst || axis > std::abs(xinputPad.sThumbLY))
				{
					xinputPad.sThumbLY = workAxis;
				}

				break;

			case XInputAxis::rightStickX:
				if (isFirst || axis > std::abs(xinputPad.sThumbRX))
				{
					xinputPad.sThumbRX = workAxis;
				}

				break;

			case XInputAxis::rightStickY:
				if (isFirst || axis > std::abs(xinputPad.sThumbRY))
				{
					xinputPad.sThumbRY = workAxis;
				}

				break;

			case XInputAxis::leftTrigger:
				if (isFirst || trigger > xinputPad.bLeftTrigger)
				{
					xinputPad.bLeftTrigger = trigger;
				}

				break;

			case XInputAxis::rightTrigger:
				if (isFirst || trigger > xinputPad.bRightTrigger)
				{
					xinputPad.bRightTrigger = trigger;
				}

				break;

			default:
				throw std::out_of_range("invalid XInputAxis");
		}
	}
}

bool InputSimulator::isOverriddenByModifierSet(const InputMapBase& map)
{
	if (modifierMaps.empty())
	{
		return false;
	}

	auto checkButtonMap = [&](InputMap const* m) -> bool
	{
		return m->isActive() &&
		       m != &map &&
		       !!(m->inputButtons.value_or(0) & map.inputButtons.value());
	};

	auto checkAxisMap = [&](InputMap const* m) -> bool
	{
		return m->isActive() &&
		       m != &map &&
		       !!(m->inputAxes.value_or(0) & map.inputAxes.value());
	};

	auto checkTouchMap = [&](InputMap const* m)
	{
		return m->isActive() && m != &map;
	};

	for (auto& pair : modifierMaps)
	{
		auto& cache = pair.second;
		cache.reset();

		if (map.inputType & InputType::button && map.inputButtons.value_or(0) != 0)
		{
			const auto buttons = map.inputButtons.value_or(0);

			for (const Ds4Buttons_t bit : Ds4Buttons_values)
			{
				if (!(bit & buttons))
				{
					continue;
				}

				auto collection = cache.getButtonMaps(bit);

				if (collection == nullptr)
				{
					continue;
				}

				if (std::any_of(collection->cbegin(), collection->cend(), checkButtonMap))
				{
					return true;
				}
			}
		}

		if (map.inputType & InputType::axis && map.inputAxes.value_or(0) != 0)
		{
			const auto axes = map.inputAxes.value();

			for (const Ds4Axes_t bit : Ds4Axes_values)
			{
				if (!(bit & axes))
				{
					continue;
				}

				auto collection = cache.getAxisMaps(bit);

				if (collection == nullptr)
				{
					continue;
				}

				if (std::any_of(collection->cbegin(), collection->cend(), checkAxisMap))
				{
					return true;
				}
			}
		}

		if (map.inputType & InputType::touchRegion && !map.inputTouchRegion.empty())
		{
			auto collection = cache.getTouchMaps(map.inputTouchRegion);

			if (collection == nullptr)
			{
				continue;
			}

			if (std::any_of(collection->cbegin(), collection->cend(), checkTouchMap))
			{
				return true;
			}
		}
	}

	return false;
}

void InputSimulator::runMap(const InputMap& m, InputModifier const* modifier)
{
	if (m.inputType == 0)
	{
		throw std::out_of_range("inputType must be non-zero.");
	}

	// UNDONE: this will not properly simulate activation for multiple input types; this is an OR, we probably want AND
	for (InputType_t value : InputType_values)
	{
		if ((m.inputType & value) == 0)
		{
			continue;
		}

		switch (value)
		{
			case InputType::button:
			{
				const PressedState state = m.simulatedState();
				applyMap(m, modifier, state, m.isActive() && ((modifier && modifier->isActive()) || m.isToggled) ? 1.0f : 0.0f);
				break;
			}

			case InputType::axis:
			{
				if (m.inputAxes.value_or(0) == 0)
				{
					throw std::invalid_argument("inputAxes has no value");
				}

				for (Ds4Axes_t bit : Ds4Axes_values)
				{
					if (!(m.inputAxes.value() & bit))
					{
						continue;
					}

					InputAxisOptions options = m.getAxisOptions(bit);

					float analog = parent->input.getAxis(m.inputAxes.value(), options.polarity);
					options.applyDeadZone(analog);

					const PressedState state = m.simulatedState();
					applyMap(m, modifier, state, analog);
				}

				break;
			}

			case InputType::touchRegion:
			{
				Ds4TouchRegion* region = touchRegions[m.inputTouchRegion];

				if (region->type == +Ds4TouchRegionType::button || !m.inputTouchDirection.has_value() || m.inputTouchDirection == Direction::none)
				{
					const PressedState state1 = handleTouchToggle(m, modifier, region->state1);
					applyMap(m, modifier, state1, Pressable::isActiveState(state1) ? 1.0f : 0.0f);

					const PressedState state2 = handleTouchToggle(m, modifier, region->state2);
					applyMap(m, modifier, state2, Pressable::isActiveState(state2) ? 1.0f : 0.0f);
				}
				else if (region->type == +Ds4TouchRegionType::trackball)
				{
					const Direction_t direction = m.inputTouchDirection.value();

					float analog = region->getSimulatedAxis(Ds4Buttons::touch1, direction);

					// TODO: update this to use the input vector-oriented dead zone application once implemented
					region->applyDeadZone(direction, analog);
					applyMap(m, modifier, m.simulatedState(), analog);
				}
				else // TODO: re-do this; Pressable::release should not be called
				{
					const Direction_t direction = m.inputTouchDirection.value();

					const float deadZone = region->getDeadZone(direction);

					PressedState state = handleTouchToggle(m, modifier, region->state1);
					float analog = region->getSimulatedAxis(Ds4Buttons::touch1, direction);

					// TODO: not this!!!
					if (analog < deadZone)
					{
						Pressable::release(state);
					}

					region->applyDeadZone(direction, analog);
					applyMap(m, modifier, state, analog);

					state = handleTouchToggle(m, modifier, region->state2);
					analog = region->getSimulatedAxis(Ds4Buttons::touch2, direction);

					// TODO: not this!!!
					if (analog < deadZone)
					{
						Pressable::release(state);
					}

					region->applyDeadZone(direction, analog);
					applyMap(m, modifier, state, analog);
				}

				return;
			}

			default:
				throw std::out_of_range("invalid InputType");
		}
	}
}

void InputSimulator::applyProfile(DeviceProfile* profile)
{
	for (auto& simulator : simulators)
	{
		simulator->deactivate(1.0f);
	}

	simulators.clear();

	touchRegions.clear();
	sortableTouchRegions.clear();

	for (auto& pair : profile->touchRegions)
	{
		touchRegions[pair.first] = &pair.second;
		sortableTouchRegions.emplace_back(&pair.second);
		addSimulator(pair.second.getSimulator(this));
	}

	bindings.clear();
	modifiers.clear();
	modifierMaps.clear();

	bindings.cache(profile->bindings, touchRegions);
	modifiers.cache(profile->modifiers, touchRegions);

	for (auto& modifier : profile->modifiers)
	{
		MapCacheCollection<InputMap> mapCache;
		mapCache.cache(modifier.bindings, touchRegions);
		modifierMaps[&modifier] = std::move(mapCache);
	}

	if (profile->useXInput)
	{
		if (!xinputConnect())
		{
			xinputDisconnect();
		}
		else
		{
			addSimulator(xinputRumbleSimulator.get());
		}
	}
	else
	{
		xinputDisconnect();
	}

	if (rumbleSequence == nullptr)
	{
		rumbleSequence = std::make_unique<RumbleSequence>(this);
	}

	const RumbleSequenceElement first = {
		RumbleSequenceBlending::none,
		125,
		0,
		1.0f
	};

	const RumbleSequenceElement second = {
		RumbleSequenceBlending::none,
		125,
		0,
		0
	};

	// we want a nice "buzz buzz", so add these both twice

	rumbleSequence->add(first);
	rumbleSequence->add(second);

	rumbleSequence->add(first);
	rumbleSequence->add(second);

	addSimulator(rumbleSequence.get());
}

// TODO: /!\ either document this thoroughly or remove it entirely
PressedState InputSimulator::handleTouchToggle(const InputMap& m, InputModifier const* modifier, const Pressable& pressable) // static
{
	if (m.inputTouchDirection.has_value() && m.inputTouchDirection != Direction::none)
	{
		return m.isToggled ? m.simulatedState() : pressable.pressedState;
	}

	if (m.rapidFire == true)
	{
		return m.simulatedState();
	}

	PressedState state = (m.isToggled || (modifier && modifier->isActive())) ? pressable.pressedState : m.pressedState;

	if (!Pressable::isActiveState(state))
	{
		state = m.simulatedState();
	}

	return state;
}

void InputSimulator::applyMap(const InputMap& m, InputModifier const* modifier, PressedState state, float analog)
{
	switch (m.simulatorType)
	{
		case SimulatorType::input:
			switch (m.outputType)
			{
				case OutputType::xinput:
					if (m.xinputButtons.has_value())
					{
						simulateXInputButton(m.xinputButtons.value(), state);
					}

					if (m.xinputAxes.has_value())
					{
						simulateXInputAxis(m.xinputAxes.value(), analog);
					}

					break;

				case OutputType::keyboard:
					simulateKeyboard(m, state);
					break;

					// TODO: AxisOptions thing for mouse
				case OutputType::mouse:
					simulateMouse(m, state, analog);
					break;

				default:
					throw std::out_of_range("invalid OutputType");
			}

			break;

		case SimulatorType::action:
			if (!m.action.has_value() || m.action.value() == +ActionType::none)
			{
				throw std::invalid_argument("action has invalid or no value");
			}

			if (m.isActive() && (modifier && modifier->isActive()))
			{
				runAction(m.action.value());
			}

			break;

		default:
			throw std::out_of_range("invalid SimulatorType");
	}
}

void InputSimulator::simulateMouse(const InputMap& m, PressedState state, float analog)
{
	if (m.mouseButton.has_value())
	{
		switch (state)
		{
			case PressedState::pressed:
				mouse.buttonDown(m.mouseButton.value());
				break;

			case PressedState::released:
				mouse.buttonUp(m.mouseButton.value());
				break;

			default:
				break;
		}
	}

	if (!m.mouseAxes.has_value())
	{
		return;
	}

	analog *= deltaTime;

	// TODO: /!\ getAxisOptions for mouse
	Direction_t direction = m.mouseAxes.value().directions;

	int x = 0;
	int y = 0;

	if ((direction & (Direction::left | Direction::right)) != (Direction::left | Direction::right))
	{
		if (direction & Direction::right)
		{
			x = static_cast<int>(analog);
		}
		else if (direction & Direction::left)
		{
			x = static_cast<int>(-analog);
		}
	}
	else
	{
		x = 0;
	}

	if ((direction & (Direction::up | Direction::down)) != (Direction::up | Direction::down))
	{
		if (direction & Direction::up)
		{
			y = static_cast<int>(-analog);
		}
		else if (direction & Direction::down)
		{
			y = static_cast<int>(analog);
		}
	}
	else
	{
		y = 0;
	}

	if (x != 0 || y != 0)
	{
		MouseSimulator::moveBy(x, y);
	}
}

void InputSimulator::simulateKeyboard(const InputMap& m, PressedState state)
{
	if (!m.keyCode.has_value())
	{
		return;
	}

	VirtualKeyCode keyCode = m.keyCode.value();

	switch (state)
	{
		case PressedState::pressed:
			keyboard.keyDown(keyCode);

			if (!m.keyCodeModifiers.empty())
			{
				for (VirtualKeyCode k : m.keyCodeModifiers)
				{
					keyboard.keyDown(k);
				}
			}

			break;

		case PressedState::released:
			keyboard.keyUp(keyCode);

			if (!m.keyCodeModifiers.empty())
			{
				for (VirtualKeyCode k : m.keyCodeModifiers)
				{
					keyboard.keyUp(k);
				}
			}

			break;

		default:
			break;
	}
}

void InputSimulator::runAction(ActionType action) const
{
	switch (action)
	{
		case ActionType::bluetoothDisconnect:
			if (parent->bluetoothConnected())
			{
				parent->disconnectBluetooth(Ds4Device::BluetoothDisconnectReason::none);
			}

			break;

		default:
			throw std::out_of_range("invalid ActionType");
	}
}

void InputSimulator::updateDeltaTime()
{
	deltaTime = static_cast<float>(duration_cast<milliseconds>(deltaStopwatch.elapsed()).count()) / deltaTimeTarget;
	deltaStopwatch.start();
}

void InputSimulator::updateModifierStates()
{
	auto visitor = [&](InputModifier* m) -> bool
	{
		return updateModifierState(*m);
	};

	for (const Ds4Buttons_t bit : Ds4Buttons_values)
	{
		modifiers.visitButtonMaps(bit, visitor);
	}

	for (const Ds4Axes_t bit : Ds4Axes_values)
	{
		modifiers.visitAxisMaps(bit, visitor);
	}

	for (auto& pair : touchRegions)
	{
		modifiers.visitTouchMaps(pair.first, visitor);
	}
}

void InputSimulator::updateBindingStates()
{
	auto visitor = [&](InputMap* m) -> bool
	{
		return updateBindingState(*m, nullptr);
	};

	for (const Ds4Buttons_t bit : Ds4Buttons_values)
	{
		bindings.visitButtonMaps(bit, visitor);
	}

	for (const Ds4Axes_t bit : Ds4Axes_values)
	{
		bindings.visitAxisMaps(bit, visitor);
	}

	for (auto& pair : touchRegions)
	{
		bindings.visitTouchMaps(pair.first, visitor);
	}
}

void InputSimulator::startTick()
{
	parent->output.leftMotor  = 0;
	parent->output.rightMotor = 0;

	bindings.reset();
	modifiers.reset();

	updateDeltaTime();
	
	simulatedXInputAxis = 0;
}

void InputSimulator::setRumble(float leftMotor, float rightMotor) const
{
	leftMotor  = std::clamp(leftMotor, 0.0f, 1.0f);
	rightMotor = std::clamp(rightMotor, 0.0f, 1.0f);

	parent->output.leftMotor  = std::max(static_cast<uint8_t>(leftMotor * 255.0f),  parent->output.leftMotor);
	parent->output.rightMotor = std::max(static_cast<uint8_t>(rightMotor * 255.0f), parent->output.rightMotor);
}

bool InputSimulator::addSimulator(ISimulator* simulator)
{
	if (simulator == nullptr)
	{
		return false;
	}

	auto it = simulators.find(simulator);

	if (it != simulators.end())
	{
		return false;
	}

	simulators.insert(simulator);
	simulator->activate(deltaTime);
	return true;
}

bool InputSimulator::removeSimulator(ISimulator* simulator)
{
	if (simulator == nullptr)
	{
		return false;
	}

	const auto it = simulators.find(simulator);

	if (it == simulators.end())
	{
		return false;
	}

	simulators.erase(it);
	return true;
}

void InputSimulator::runSimulators()
{
	for (auto it = simulators.begin(); it != simulators.end();)
	{
		ISimulator* ptr = *it;
		ptr->update(deltaTime);

		if (ptr->state == SimulatorState::inactive)
		{
			ptr->deactivate(deltaTime);
			it = simulators.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void InputSimulator::runMaps()
{
	startTick();

	updateTouchRegions();
	updateModifierStates();
	updateBindingStates();

	runSimulators();

	if (parent->profile.useXInput && realXInputIndex >= 0 && xinputPad != xinputLast)
	{
		xinputLast = xinputPad;
		xinputTarget->update(xinputPad);
	}
}

void InputSimulator::runPersistent()
{
	startTick();

	for (InputModifier* modifier : modifiers.allMaps())
	{
		if (modifier->isPersistent())
		{
			updateModifierState(*modifier);
		}
	}

	for (InputMap* map : bindings.allMaps())
	{
		if (map->isPersistent())
		{
			updateBindingState(*map, nullptr);
		}
	}

	runSimulators();
}

void InputSimulator::updateTouchRegions()
{
	Ds4Buttons_t disallow = 0;

	std::sort(sortableTouchRegions.begin(), sortableTouchRegions.end(), [](const Ds4TouchRegion* a, const Ds4TouchRegion* b)
	{
		return (a->isTouchActive(touchMask) && !a->allowCrossOver) && !(b->isTouchActive(touchMask) && !b->allowCrossOver);
	});

	const Ds4Buttons_t held = parent->input.heldButtons;
	const Ds4Buttons_t inactive = (held & touchMask) ^ touchMask;

	auto makeInactive = [&](Ds4Buttons_t touchId, Ds4TouchRegion* region, const Ds4Vector2& point)
	{
		if ((inactive & touchId) && region->isTouchActive(touchId))
		{
			region->deactivateTouch(touchId, point);
		}
	};

	// TODO: devise a cleaner way to get and manipulate touch data for each touch point

	for (auto& region : sortableTouchRegions)
	{
		if (region->isTouchActive(inactive))
		{
			makeInactive(Ds4Buttons::touch1, region, parent->input.data.touchPoint1);
			makeInactive(Ds4Buttons::touch2, region, parent->input.data.touchPoint2);
			continue;
		}

		updateTouchRegion(*region, Ds4Buttons::touch1, parent->input.data.touchPoint1, disallow);
		updateTouchRegion(*region, Ds4Buttons::touch2, parent->input.data.touchPoint2, disallow);
	}
}

void InputSimulator::updateTouchRegion(Ds4TouchRegion& region, Ds4Buttons_t sender, const Ds4Vector2& point, Ds4Buttons_t& disallow) const
{
	if (!!(disallow & sender) || !(parent->input.heldButtons & sender) || !region.isInRegion(sender, point))
	{
		region.deactivateTouch(sender, point);
		return;
	}

	region.activateTouch(sender, point);

	if (!region.allowCrossOver)
	{
		disallow |= sender;
	}
}

void InputSimulator::updatePressedState(InputMapBase& instance, const std::function<void()>& press, const std::function<void()>& release)
{
	if (isOverriddenByModifierSet(instance))
	{
		release();
		return;
	}

	for (InputType_t value : InputType_values)
	{
		if ((instance.inputType & value) == 0)
		{
			continue;
		}

		switch (value)
		{
			case InputType::button:
				if ((instance.inputButtons.value() & parent->input.heldButtons) == instance.inputButtons)
				{
					press();
				}
				else
				{
					release();
				}

				break;

			case InputType::axis:
			{
				if (instance.inputAxes.value_or(0) == 0)
				{
					throw std::invalid_argument("inputAxes has invalid or no value");
				}

				const gsl::span<const Ds4Axes_t> axes(Ds4Axes_values);

				const size_t target = std::count_if(axes.begin(), axes.end(), [&](Ds4Axes_t x) -> bool
				{
					return (x & instance.inputAxes.value()) != 0;
				});

				size_t count = 0;

				for (Ds4Axes_t bit : Ds4Axes_values)
				{
					if (!(instance.inputAxes.value() & bit))
					{
						continue;
					}

					InputAxisOptions options = instance.getAxisOptions(bit);

					float axis = parent->input.getAxis(instance.inputAxes.value(), options.polarity);

					if (axis >= options.deadZone.value_or(0.0f))
					{
						++count;
					}
					else
					{
						break;
					}
				}

				if (count == target)
				{
					press();
				}
				else
				{
					release();
				}

				break;
			}

			case InputType::touchRegion:
			{
				auto it = touchRegions.find(instance.inputTouchRegion);

				if (it == touchRegions.end())
				{
					break;
				}

				Ds4TouchRegion* region = it->second;
				const auto direction = instance.inputTouchDirection.value_or(Direction::none);

				if (region->isActive(Ds4Buttons::touch1, direction) ||
				    region->isActive(Ds4Buttons::touch2, direction))
				{
					press();
				}
				else
				{
					release();
				}

				break;
			}

			case InputType::none:
				break;

			default:
				throw std::out_of_range("invalid InputType");
		}
	}
}

bool InputSimulator::updateModifierState(InputModifier& modifier)
{
	const PressedState oldPressedState = modifier.pressedState;

	const auto press = [&]() -> void
	{
		modifier.press();
	};

	const auto release = [&]() -> void
	{
		modifier.release();
	};

	updatePressedState(modifier, press, release);

	if (!modifier.bindings.empty())
	{
		for (InputMap& bind : modifier.bindings)
		{
			updateBindingState(bind, &modifier);
		}
	}

	return oldPressedState != modifier.pressedState;
}

bool InputSimulator::updateBindingState(InputMap& map, InputModifier* modifier)
{
	const PressedState oldPressedState = map.pressedState;

	if (modifier != nullptr && map.toggle != true && !modifier->isActive())
	{
		map.release();
		runMap(map, modifier);
		return map.pressedState != oldPressedState;
	}

	const auto press = [&]() -> void
	{
		map.pressWithModifier(modifier);
	};

	const auto release = [&]() -> void
	{
		map.release();
	};

	updatePressedState(map, press, release);
	runMap(map, modifier);
	return oldPressedState != map.pressedState;
}

bool InputSimulator::xinputConnect()
{
	if (!xinputDeviceOpen())
	{
		return false;
	}

	if (realXInputIndex >= 0)
	{
		return true;
	}

	//int index = profile->autoXInputIndex ? ScpDevice::getFreePort() : profile->xinputIndex;

	if (VIGEM_SUCCESS(xinputTarget->connect()))
	{
		// TODO: get LED index from ViGEm
		realXInputIndex = 0;
		return true;
	}

	// If connecting an emulated XInput controller failed,
	// it's likely because it's already connected. Disconnect
	// it before continuing.
	xinputTarget->disconnect();

	bool ok = false;

	// Try up to 4 times to recover the virtual controller.
	for (size_t i = 0; i < 4; i++)
	{
		ok = xinputTarget->connect();

		if (ok)
		{
			break;
		}

		std::this_thread::yield();
		std::this_thread::sleep_for(250ms);
	}

	if (!ok)
	{
		onXInputHandleFailure.invoke(parent);
		return false;
	}

	realXInputIndex = 0;
	return true;
}

void InputSimulator::xinputDisconnect()
{
	if (realXInputIndex < 0 || !xinputTarget)
	{
		return;
	}

	xinputTarget->disconnect();
	realXInputIndex = -1;
}

bool InputSimulator::xinputDeviceOpen()
{
	if (!Program::driver.isOpen())
	{
		return false;
	}

	if (xinputTarget == nullptr)
	{
		xinputTarget = std::make_shared<vigem::XInputTarget>(&Program::driver);

		xinputRumbleSimulator = std::make_unique<XInputRumbleSimulator>(this);
		xinputRumbleSimulator->xinputTarget = xinputTarget;
	}

	return true;
}

void InputSimulator::xinputDeviceClose()
{
	if (xinputRumbleSimulator)
	{
		removeSimulator(xinputRumbleSimulator.get());
		xinputRumbleSimulator = nullptr;
	}

	if (xinputTarget)
	{
		xinputTarget->disconnect();
	}

	xinputTarget = nullptr;
}
