#include "stdafx.h"

#include <chrono>

#include "Ds4Device.h"
#include "InputSimulator.h"

using namespace std::chrono;

std::recursive_mutex InputSimulator::s_scpLock {};
std::shared_ptr<ScpDevice> InputSimulator::s_scpDevice = nullptr;

InputSimulator::InputSimulator(Ds4Device* parent)
	: parent(parent)
{
	scpDeviceOpen();
	this->scpDevice = s_scpDevice;
}

InputSimulator::~InputSimulator()
{
	scpDisconnect();
}

void InputSimulator::simulateXInputButton(XInputButtons_t buttons, PressedState state)
{
	XInputButtons_t dest = xpad.wButtons;

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
			throw /* TODO new ArgumentOutOfRangeException(nameof(state), state, "Invalid pressed state.")*/;
	}

	xpad.wButtons = dest;
}

void InputSimulator::simulateXInputAxis(XInputAxes& axes, float m)
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
				if (isFirst || axis > std::abs(xpad.sThumbLX))
				{
					xpad.sThumbLX = workAxis;
				}

				break;

			case XInputAxis::leftStickY:
				if (isFirst || axis > std::abs(xpad.sThumbLY))
				{
					xpad.sThumbLY = workAxis;
				}

				break;

			case XInputAxis::rightStickX:
				if (isFirst || axis > std::abs(xpad.sThumbRX))
				{
					xpad.sThumbRX = workAxis;
				}

				break;

			case XInputAxis::rightStickY:
				if (isFirst || axis > std::abs(xpad.sThumbRY))
				{
					xpad.sThumbRY = workAxis;
				}

				break;

			case XInputAxis::leftTrigger:
				if (isFirst || trigger > xpad.bLeftTrigger)
				{
					xpad.bLeftTrigger = trigger;
				}

				break;

			case XInputAxis::rightTrigger:
				if (isFirst || trigger > xpad.bRightTrigger)
				{
					xpad.bRightTrigger = trigger;
				}

				break;

			default:
				throw /* TODO: new ArgumentOutOfRangeException(nameof(options), options, "Invalid XInput axis.")*/;
		}
	}
}

// TODO: OPTIMIZE !!!
void InputSimulator::getActive(InputMapBase& map)
{
	if (!map_cache.empty())
	{
		return;
	}

	for (InputModifier& mod : profile->modifiers)
	{
		if (!mod.isActive())
		{
			continue;
		}

		for (InputMap& binding : mod.bindings)
		{
			if (&binding != &map)
			{
				map_cache.push_back(&binding);
			}
		}
	}
}

bool InputSimulator::isOverriddenByModifierSet(InputMapBase& map)
{
	if (profile->modifiers.empty())
	{
		return false;
	}

	map_cache.clear();

	/*maps = Profile.Modifiers
		.Where(x = > x.IsActive)
		.Select(y = > y.Bindings)
		.Where(x = > x != nullptr)
		.SelectMany(x = > x)
		.Where(x = > !ReferenceEquals(x, map))
		.ToList();*/

	if (map.inputType & InputType::button)
	{
		getActive(map);

		if (std::any_of(map_cache.begin(), map_cache.end(), [&](InputMap* x) -> bool
		{
			return (x->inputType & InputType::button) != 0 && (x->inputButtons.value_or(0) & map.inputButtons.value_or(0)) != 0;
		}))
		{
			return true;
		}
	}

	if (map.inputType & InputType::axis)
	{
		getActive(map);

		if (std::any_of(map_cache.begin(), map_cache.end(), [&](InputMap* x) -> bool
		{
			return (x->inputType & InputType::axis) != 0 && (x->inputAxis.value_or(0) & map.inputAxis.value_or(0)) != 0;
		}))
		{
			return true;
		}
	}

	if (map.inputType & InputType::touchRegion)
	{
		getActive(map);

		if (std::any_of(map_cache.begin(), map_cache.end(), [&](InputMap* x) -> bool
		{
			return (x->inputType & InputType::touchRegion) != 0 && x->inputRegion == map.inputRegion;
		}))
		{
			return true;
		}
	}

	return false;
}

void InputSimulator::runMap(InputMap& m, InputModifier* modifier)
{
	if (m.inputType == 0)
	{
		throw /* TODO: new ArgumentOutOfRangeException(nameof(m.inputType), m.inputType, "InputType must be non-zero.")*/;
	}

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
				PressedState state = m.simulatedState();
				applyMap(m, modifier, state, m.isActive() && ((modifier && modifier->isActive()) || m.isToggled) ? 1.0f : 0.0f);
				break;
			}

			case InputType::axis:
			{
				if (m.inputAxis.value_or(0) == 0)
				{
					throw /* TODO: new ArgumentNullException(nameof(m.InputAxis))*/;
				}

				for (Ds4Axis_t bit : Ds4Axis_values)
				{
					if (!(m.inputAxis.value() & bit))
					{
						continue;
					}

					InputAxisOptions options = m.getAxisOptions(bit);

					float analog = parent->input.getAxis(m.inputAxis.value(), options.polarity);
					options.applyDeadZone(analog);

					PressedState state = m.simulatedState();
					applyMap(m, modifier, state, analog);
				}

				break;
			}

			case InputType::touchRegion:
			{
				Ds4TouchRegion& region = profile->touchRegions[m.inputRegion];

				if (region.type == +Ds4TouchRegionType::button || !m.touchDirection.has_value() || m.touchDirection == Direction::none)
				{
					PressedState state = handleTouchToggle(m, modifier, region.state1);
					applyMap(m, modifier, state, Pressable::isActiveState(state) ? 1.0f : 0.0f);

					state = handleTouchToggle(m, modifier, region.state2);
					applyMap(m, modifier, state, Pressable::isActiveState(state) ? 1.0f : 0.0f);
				}
				else
				{
					Direction_t direction = m.touchDirection.value();

					float deadZone = region.getDeadZone(direction);

					PressedState state = handleTouchToggle(m, modifier, region.state1);
					float analog = region.getTouchDelta(Ds4Buttons::touch1, direction, parent->input.data.touchPoint1);

					if (analog < deadZone)
					{
						Pressable::release(state);
					}

					region.applyDeadZone(direction, analog);
					applyMap(m, modifier, state, analog);

					state = handleTouchToggle(m, modifier, region.state2);
					analog = region.getTouchDelta(Ds4Buttons::touch2, direction, parent->input.data.touchPoint2);

					if (analog < deadZone)
					{
						Pressable::release(state);
					}

					region.applyDeadZone(direction, analog);
					applyMap(m, modifier, state, analog);
				}

				return;
			}

			default:
				throw /* TODO: new ArgumentOutOfRangeException()*/;
		}
	}
}

void InputSimulator::applyProfile(DeviceProfile* profile)
{
	this->profile = profile;

	touchRegions.clear();

	for (auto& pair : profile->touchRegions)
	{
		touchRegions.push_back(&pair.second);
	}

	if (profile->useXInput)
	{
		if (!scpConnect())
		{
			scpDisconnect();
		}
	}
	else
	{
		scpDisconnect();
	}
}

void InputSimulator::readXInput() const
{
	if (!profile->useXInput || scpDevice == nullptr)
	{
		return;
	}

	if (realXInputIndex < 0)
	{
		return;
	}

	scpDevice->syncState(realXInputIndex);
	parent->output.fromXInput(realXInputIndex, scpDevice.get());
}

PressedState InputSimulator::handleTouchToggle(InputMap& m, InputModifier* modifier, const Pressable& pressable)
{
	if (m.touchDirection.has_value() && m.touchDirection != Direction::none)
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

void InputSimulator::applyMap(InputMap& m, InputModifier* modifier, PressedState state, float analog)
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
					throw /* TODO: new ArgumentOutOfRangeException(nameof(m.outputType), m.outputType, "Invalid output type.")*/;
			}

			break;

		case SimulatorType::action:
			if (!m.action.has_value() || m.action.value() == +ActionType::none)
			{
				throw /* TODO: new ArgumentNullException(nameof(m.action))*/;
			}

			if (m.isActive() && (modifier && modifier->isActive()))
			{
				runAction(m.action.value());
			}

			break;

		default:
			throw /* TODO: new ArgumentOutOfRangeException(nameof(m.simulatorType), m.simulatorType, "Invalid map type.")*/;
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

	analog *= parent->deltaTime;

	// TODO: /!\ actually the thing (GetAxisOptions)
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
			throw /* TODO: new ArgumentOutOfRangeException(nameof(action), action, "Invalid action type.")*/;
	}
}

void InputSimulator::runMaps()
{
	simulatedXInputAxis = 0;

	for (InputModifier& modifier : profile->modifiers)
	{
		updatePressedState(modifier);
	}

	if (parent->input.touchChanged)
	{
		updateTouchRegions();
	}

	for (InputMap& m : profile->bindings)
	{
		updateBindingState(m, nullptr);
	}

	if (profile->useXInput && realXInputIndex >= 0 && xpad != last_xpad)
	{
		last_xpad = xpad;
		scpDevice->syncState(realXInputIndex, xpad);
	}
}

void InputSimulator::runPersistent()
{
	simulatedXInputAxis = 0;

	for (InputModifier& modifier : profile->modifiers)
	{
		if (modifier.isPersistent())
		{
			updatePressedState(modifier);
		}
	}

	for (InputMap& m : profile->bindings)
	{
		if (m.isPersistent())
		{
			updateBindingState(m, nullptr);
		}
	}
}

void InputSimulator::updateTouchRegions()
{
	Ds4Buttons_t disallow = 0;

	// foreach (Ds4TouchRegion region in Profile.TouchRegions.Values.OrderBy(x = > !x.IsActive(touchMask) && !x.AllowCrossOver))

	std::sort(touchRegions.begin(), touchRegions.end(), [](const Ds4TouchRegion* a, const Ds4TouchRegion* b)
	{
		return (a->isActive(touchMask) && !a->allowCrossOver) && !(b->isActive(touchMask) && !b->allowCrossOver);
	});

	for (auto& region : touchRegions)
	{
		if (!(parent->input.heldButtons & touchMask))
		{
			region->setInactive(touchMask);
			continue;
		}

		updateTouchRegion(*region, /* TODO */ nullptr, Ds4Buttons::touch1, parent->input.data.touchPoint1, disallow);
		updateTouchRegion(*region, /* TODO */ nullptr, Ds4Buttons::touch2, parent->input.data.touchPoint2, disallow);
	}
}

void InputSimulator::updateTouchRegion(Ds4TouchRegion& region, InputModifier* modifier, Ds4Buttons_t sender, Ds4Vector2& point, Ds4Buttons_t& disallow)
{
	if (!(disallow & sender) && (parent->input.heldButtons & sender))
	{
		if (region.isInRegion(sender, point))
		{
			if (modifier && !modifier->isActive())
			{
				updatePressedState(*modifier);
			}

			if (!modifier || modifier->isActive())
			{
				region.setActive(sender, point);

				if (!region.allowCrossOver)
				{
					disallow |= sender;
				}

				return;
			}
		}
	}

	region.setInactive(sender);
}

void InputSimulator::updatePressedStateImpl(InputMapBase& instance, const std::function<void()>& press, const std::function<void()>& release)
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
				if (!instance.inputAxis.has_value() || instance.inputAxis.value() == 0)
				{
					throw /* TODO: new ArgumentNullException(nameof(instance.InputAxis))*/;
				}

				const gsl::span<const Ds4Axis_t> s(Ds4Axis_values);

				const size_t target = std::count_if(s.begin(), s.end(), [&](Ds4Axis_t x) -> bool
				{
					return (x & instance.inputAxis.value_or(0)) != 0;
				});
				size_t count = 0;

				for (Ds4Axis_t bit : Ds4Axis_values)
				{
					if (!(instance.inputAxis.value() & bit))
					{
						continue;
					}

					InputAxisOptions options = instance.getAxisOptions(bit);

					float axis = parent->input.getAxis(instance.inputAxis.value(), options.polarity);

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
				auto it = profile->touchRegions.find(instance.inputRegion);
				if (it == profile->touchRegions.end())
				{
					break;
				}

				auto& region = it->second;

				if (region.isActive(touchMask))
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
				throw; // TODO
		}
	}
}

void InputSimulator::updatePressedState(InputModifier& modifier)
{
	const auto a = [&]() -> void
	{
		modifier.press();
	};

	const auto b = [&]() -> void
	{
		modifier.release();
	};

	updatePressedStateImpl(modifier, a, b);

	if (modifier.bindings.empty())
	{
		return;
	}

	for (InputMap& bind : modifier.bindings)
	{
		updateBindingState(bind, &modifier);
	}
}

void InputSimulator::updatePressedState(InputMap& map, InputModifier* modifier)
{
	const auto a = [&]() -> void
	{
		map.pressModifier(modifier);
	};

	const auto b = [&]() -> void
	{
		map.release();
	};

	updatePressedStateImpl(map, a, b);
}

void InputSimulator::updateBindingState(InputMap& m, InputModifier* modifier)
{
	if (modifier != nullptr)
	{
		if (m.toggle != true)
		{
			if (!modifier->isActive())
			{
				m.release();
				runMap(m, modifier);
				return;
			}
		}
	}

	switch (m.inputType)
	{
		case InputType::touchRegion:
		{
			auto it = profile->touchRegions.find(m.inputRegion);
			if (it == profile->touchRegions.end())
			{
				break;
			}

			if (m.touchDirection == Direction::none)
			{
				if (it->second.isActive(touchMask))
				{
					m.pressModifier(modifier);
				}
				else
				{
					m.release();
				}
			}

			break;
		}

		default:
			updatePressedState(m, modifier);
			break;
	}

	runMap(m, modifier);
}

bool InputSimulator::scpConnect()
{
	if (!scpDeviceOpen())
	{
		return false;
	}

	if (scpDevice == nullptr)
	{
		scpDevice = s_scpDevice;
	}

	if (realXInputIndex >= 0)
	{
		return true;
	}

	int index = profile->autoXInputIndex ? ScpDevice::getFreePort() : profile->xinputIndex;

	if (index < 0)
	{
		return false;
	}

	if (scpDevice->connect(index))
	{
		realXInputIndex = index;
		return true;
	}

	// If connecting an emulated XInput controller failed,
	// it's likely because it's already connected. Disconnect
	// it before continuing.
	scpDevice->disconnect(index);

	bool ok = false;

	// Try up to 4 times to recover the virtual controller.
	for (size_t i = 0; i < 4; i++)
	{
		ok = scpDevice->connect(index);

		if (ok)
		{
			break;
		}

		std::this_thread::yield();
		std::this_thread::sleep_for(250ms);
	}

	if (!ok)
	{
		onScpXInputHandleFailure.invoke(parent);
		return false;
	}

	realXInputIndex = index;
	return true;
}

void InputSimulator::scpDisconnect()
{
	if (realXInputIndex < 0 || !scpDevice)
	{
		return;
	}

	scpDevice->disconnect(realXInputIndex);
	realXInputIndex = -1;
}

bool InputSimulator::scpDeviceOpen()
{
	std::lock_guard<std::recursive_mutex> lock(s_scpLock);

	if (s_scpDevice != nullptr)
	{
		return true;
	}

	std::unique_ptr<hid::HidInstance> info;

	hid::enumerateGuid([&](const std::wstring& path, const std::wstring& instanceId) -> bool
	{
		info = std::make_unique<hid::HidInstance>(path, instanceId);
		return true;
	}, GUID_DEVINTERFACE_SCPVBUS);

	if (info == nullptr)
	{
		// TODO: onScpDeviceMissing.invoke(this);
		return false;
	}

	auto handle = Handle(CreateFile(info->path.c_str(), GENERIC_READ | GENERIC_WRITE,
	                                FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr), true);

	if (!handle.isValid())
	{
		// TODO: onScpDeviceOpenFailed.invoke(this);
		return false;
	}

	s_scpDevice = std::make_shared<ScpDevice>(std::move(handle));
	return true;
}

void InputSimulator::scpDeviceClose()
{
	if (s_scpDevice == nullptr)
	{
		return;
	}

	s_scpDevice = nullptr;
}
