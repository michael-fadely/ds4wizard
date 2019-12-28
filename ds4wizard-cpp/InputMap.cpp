#include "pch.h"
#include "InputMap.h"
#include <utility>

bool InputMapBase::performRapidFire() const
{
	return rapidStopwatch.elapsed() >= rapidFireInterval;
}

PressedState InputMapBase::simulatedState() const
{
	if (rapidFire == true)
	{
		return rapidState;
	}

	if (toggle != true)
	{
		return pressedState;
	}

	if (isActiveState(pressedState))
	{
		return pressedState;
	}

	return isActive() ? PressedState::on : pressedState;
}

bool InputMapBase::isActive() const
{
	{
		if (toggle == true)
		{
			return isToggled;
		}

		return Pressable::isActive();
	}
}

bool InputMapBase::isPersistent() const
{
	return rapidFire == true;
}

InputMapBase::InputMapBase(const InputMapBase& other)
	: Pressable(other),
	  inputType(other.inputType),
	  inputButtons(other.inputButtons),
	  inputAxes(other.inputAxes),
	  inputTouchRegion(other.inputTouchRegion),
	  toggle(other.toggle),
	  rapidFire(other.rapidFire),
	  rapidFireInterval(other.rapidFireInterval),
	  inputAxisOptions(other.inputAxisOptions)
{
}

InputMapBase::InputMapBase(InputMapBase&& other) noexcept
	: Pressable(std::move(other)),
	  inputType(other.inputType),
	  inputButtons(other.inputButtons),
	  inputAxes(other.inputAxes),
	  inputTouchRegion(std::move(other.inputTouchRegion)),
	  toggle(other.toggle),
	  rapidFire(other.rapidFire),
	  rapidFireInterval(other.rapidFireInterval),
	  inputAxisOptions(std::move(other.inputAxisOptions))
{
}

InputMapBase::InputMapBase(InputType_t inputType)
	: inputType(inputType)
{
}

InputMapBase::InputMapBase(InputType_t inputType, Ds4Buttons::T input)
	: inputType(inputType),
	  inputButtons(input)
{
}

InputMapBase::InputMapBase(InputType_t inputType, Ds4Axes::T input)
	: inputType(inputType),
	  inputAxes(input)
{
}

InputMapBase::InputMapBase(InputType_t inputType, std::string input)
	: inputType(inputType),
	  inputTouchRegion(std::move(input))
{
}

InputMapBase& InputMapBase::operator=(InputMapBase&& other) noexcept
{
	Pressable::operator=(std::move(other));

	inputType         = other.inputType;
	inputButtons      = other.inputButtons;
	inputAxes         = other.inputAxes;
	inputTouchRegion  = std::move(other.inputTouchRegion);
	toggle            = other.toggle;
	rapidFire         = other.rapidFire;
	rapidFireInterval = other.rapidFireInterval;
	inputAxisOptions  = std::move(other.inputAxisOptions);

	return *this;
}

void InputMapBase::press()
{
	if (toggle == true && pressedState == PressedState::pressed)
	{
		isToggled = !isToggled;
	}

	Pressable::press();

	if (rapidFire == true)
	{
		updateRapidState();
	}
}

void InputMapBase::updateRapidState()
{
	if (isActive())
	{
		if (!rapidStopwatch.running())
		{
			rapidStopwatch.start();
		}
	}
	else
	{
		if (rapidStopwatch.running())
		{
			rapidStopwatch.stop();
			//rapidStopwatch.reset();
			rapidStopwatch.start();
		}

		rapidFiring = false;
	}

	if (rapidFiring)
	{
		if (performRapidFire())
		{
			rapidFiring = false;
			Pressable::release(rapidState);
			rapidStopwatch.start();
		}
		else
		{
			Pressable::press(rapidState);
		}
	}
	else
	{
		if (performRapidFire())
		{
			rapidFiring = true;
			Pressable::press(rapidState);
			rapidStopwatch.start();
		}
		else
		{
			Pressable::release(rapidState);
		}
	}
}

void InputMapBase::release()
{
	Pressable::release();

	if (rapidFire == true)
	{
		updateRapidState();
	}
}

InputAxisOptions InputMapBase::getAxisOptions(Ds4Axes_t axis)
{
	if (inputAxisOptions.empty())
	{
		// HACK: remove
		return InputAxisOptions();
	}

	const auto it = inputAxisOptions.find(axis);

	if (it != inputAxisOptions.end())
	{
		return it->second;
	}

	// HACK: remove
	return InputAxisOptions();
}

bool InputMapBase::operator==(const InputMapBase& other) const
{
	return inputType == other.inputType
	       && inputButtons == other.inputButtons
	       && inputAxes == other.inputAxes
	       && inputTouchRegion == other.inputTouchRegion
	       && toggle == other.toggle
	       && rapidFire == other.rapidFire
	       && rapidFireInterval == other.rapidFireInterval
	       && inputAxisOptions == other.inputAxisOptions;
}

bool InputMapBase::operator!=(const InputMapBase& other) const
{
	return !(*this == other);
}

void InputMapBase::readJson(const nlohmann::json& json)
{
	if (json.find("inputType") != json.end())
	{
		InputType_t inputType_;
		ENUM_DESERIALIZE_FLAGS(InputType)(json["inputType"].get<std::string>(), inputType_);
		inputType = inputType_;
	}

	if (json.find("inputButtons") != json.end())
	{
		Ds4Buttons_t inputButtons_;
		ENUM_DESERIALIZE_FLAGS(Ds4Buttons)(json["inputButtons"].get<std::string>(), inputButtons_);
		inputButtons = inputButtons_;
	}

	if (json.find("inputAxes") != json.end())
	{
		Ds4Axes_t inputAxis_;
		ENUM_DESERIALIZE_FLAGS(Ds4Axes)(json["inputAxes"].get<std::string>(), inputAxis_);
		inputAxes = inputAxis_;
	}

	if (json.find("inputTouchRegion") != json.end())
	{
		inputTouchRegion = json["inputTouchRegion"];
	}

	if (json.find("toggle") != json.end())
	{
		toggle = json["toggle"];
	}

	if (json.find("rapidFire") != json.end())
	{
		rapidFire = json["rapidFire"];
	}

	if (json.find("rapidFireInterval") != json.end())
	{
		rapidFireInterval = std::chrono::microseconds(json["rapidFireInterval"].get<int64_t>());
	}

	if (json.find("inputAxisOptions") != json.end())
	{
		auto inputAxisOptions_ = json["inputAxisOptions"].items();

		for (const auto& pair : inputAxisOptions_)
		{
			Ds4Axes_t flags;
			ENUM_DESERIALIZE_FLAGS(Ds4Axes)(pair.key(), flags);
			inputAxisOptions[flags] = fromJson<InputAxisOptions>(pair.value());
		}
	}
}

void InputMapBase::writeJson(nlohmann::json& json) const
{
	json["inputType"] = ENUM_SERIALIZE_FLAGS(InputType)(inputType).c_str();

	if (inputButtons.has_value())
	{
		json["inputButtons"] = ENUM_SERIALIZE_FLAGS(Ds4Buttons)(inputButtons.value()).c_str();
	}

	if (inputAxes.has_value())
	{
		json["inputAxes"] = ENUM_SERIALIZE_FLAGS(Ds4Axes)(inputAxes.value()).c_str();
	}

	if (inputTouchRegion.length())
	{
		json["inputTouchRegion"] = inputTouchRegion.c_str();
	}

	if (toggle.has_value())
	{
		json["toggle"] = toggle.value();
	}

	if (rapidFire.has_value())
	{
		json["rapidFire"] = rapidFire.value();
	}

	if (rapidFireInterval.has_value())
	{
		json["rapidFireInterval"] = rapidFireInterval.value().count();
	}

	nlohmann::json inputAxisOptions_;

	for (const auto& pair : inputAxisOptions)
	{
		auto key = ENUM_SERIALIZE_FLAGS(Ds4Axes)(pair.first);
		inputAxisOptions_[key] = pair.second.toJson();
	}

	json["inputAxisOptions"] = inputAxisOptions_;
}

InputModifier::InputModifier(InputType_t type, Ds4Buttons::T buttons)
	: InputMapBase(type, buttons)
{
}

InputModifier::InputModifier(InputType_t type, Ds4Axes::T axis)
	: InputMapBase(type, axis)
{
}

InputModifier::InputModifier(InputType_t type, const std::string& region)
	: InputMapBase(type, region)
{
}

InputModifier::InputModifier(const InputModifier& other)
	: InputMapBase(other)
{
	if (!other.bindings.empty())
	{
		bindings = other.bindings;
	}
}

InputModifier::InputModifier(InputModifier&& other) noexcept
	: InputMapBase(std::move(other)),
	  bindings(std::move(other.bindings))
{
}

InputModifier& InputModifier::operator=(InputModifier&& other) noexcept
{
	InputMapBase::operator=(std::move(other));
	bindings = std::move(other.bindings);
	return *this;
}

bool InputModifier::operator==(const InputModifier& other) const
{
	return InputMapBase::operator==(other) && bindings == other.bindings;
}

bool InputModifier::operator!=(const InputModifier& other) const
{
	return !(*this == other);
}

void InputModifier::readJson(const nlohmann::json& json)
{
	InputMapBase::readJson(json);

	for (const auto& value : json["bindings"])
	{
		bindings.push_back(fromJson<InputMap>(value));
	}
}

void InputModifier::writeJson(nlohmann::json& json) const
{
	InputMapBase::writeJson(json);

	nlohmann::json bindings_;

	for (const auto& binding : bindings)
	{
		bindings_.push_back(binding.toJson());
	}

	json["bindings"] = bindings_;
}

InputMap::InputMap(InputMap&& other) noexcept
	: InputMapBase(std::move(other)),
	  simulatorType(other.simulatorType),
	  outputType(other.outputType),
	  action(other.action),
	  touchDirection(other.touchDirection),
	  keyCode(other.keyCode),
	  keyCodeModifiers(std::move(other.keyCodeModifiers)),
	  mouseAxes(other.mouseAxes),
	  mouseButton(other.mouseButton),
	  xinputButtons(other.xinputButtons),
	  xinputAxes(other.xinputAxes)
{
}

InputMap::InputMap(SimulatorType simulatorType, InputType_t inputType, OutputType::T outputType)
	: InputMapBase(inputType),
	  simulatorType(simulatorType),
	  outputType(outputType)
{
}

InputMap& InputMap::operator=(InputMap&& other) noexcept
{
	InputMapBase::operator=(std::move(other));

	simulatorType    = other.simulatorType;
	outputType       = other.outputType;
	action           = other.action;
	touchDirection   = other.touchDirection;
	keyCode          = other.keyCode;
	keyCodeModifiers = std::move(other.keyCodeModifiers);
	mouseAxes        = other.mouseAxes;
	mouseButton      = other.mouseButton;
	xinputButtons    = other.xinputButtons;
	xinputAxes       = other.xinputAxes;

	return *this;
}

void InputMap::pressWithModifier(const InputModifier* modifier)
{
	if (!modifier || modifier->isActive())
	{
		InputMapBase::press();
	}
	else if (toggle.value_or(false) && rapidFire.value_or(false))
	{
		updateRapidState();
	}
}

bool InputMap::operator==(const InputMap& other) const
{
	return InputMapBase::operator==(other)
	       && simulatorType == other.simulatorType
	       && outputType == other.outputType
	       && action == other.action
	       && keyCode == other.keyCode
	       && keyCodeModifiers == other.keyCodeModifiers
	       && xinputButtons == other.xinputButtons
	       && xinputAxes == other.xinputAxes
	       && mouseAxes == other.mouseAxes
	       && touchDirection == other.touchDirection
	       && mouseButton == other.mouseButton;
}

bool InputMap::operator!=(const InputMap& other) const
{
	return !(*this == other);
}

void InputMap::readJson(const nlohmann::json& json)
{
	InputMapBase::readJson(json);

	ENUM_DESERIALIZE_FLAGS(OutputType)(json["outputType"].get<std::string>(), outputType);

	if (json.find("xinputButtons") != json.end())
	{
		XInputButtons_t xinputButtons_;
		ENUM_DESERIALIZE_FLAGS(XInputButtons)(json["xinputButtons"].get<std::string>(), xinputButtons_);
		xinputButtons = xinputButtons_;
	}

	if (json.find("touchDirection") != json.end())
	{
		Direction_t touchDirection_;
		ENUM_DESERIALIZE_FLAGS(Direction)(json["touchDirection"].get<std::string>(), touchDirection_);
		touchDirection = touchDirection_;
	}

	if (json.find("simulatorType") != json.end())
	{
		simulatorType = SimulatorType::_from_string(json.value("simulatorType", "none").c_str());
	}

	if (json.find("action") != json.end())
	{
		action = ActionType::_from_string(json.value("action", "none").c_str());
	}

	if (json.find("keyCode") != json.end())
	{
		keyCode = json["keyCode"].get<VirtualKeyCode>();
	}

	if (json.find("keyCodeModifiers") != json.end())
	{
		keyCodeModifiers = json["keyCodeModifiers"].get<std::vector<VirtualKeyCode>>();
	}

	if (json.find("mouseAxes") != json.end())
	{
		mouseAxes = fromJson<MouseAxes>(json["mouseAxes"]);
	}

	if (json.find("mouseButtons") != json.end())
	{
		mouseButton = MouseButton::_from_string(json.value("mouseButton", "").c_str());
	}

	if (json.find("xinputAxes") != json.end())
	{
		xinputAxes = fromJson<XInputAxes>(json["xinputAxes"]);
	}
}

void InputMap::writeJson(nlohmann::json& json) const
{
	InputMapBase::writeJson(json);

	json["outputType"] = ENUM_SERIALIZE_FLAGS(OutputType)(outputType).c_str();

	if (xinputButtons.has_value())
	{
		json["xinputButtons"] = ENUM_SERIALIZE_FLAGS(XInputButtons)(xinputButtons.value()).c_str();
	}

	if (touchDirection.has_value())
	{
		json["touchDirection"] = ENUM_SERIALIZE_FLAGS(Direction)(touchDirection.value()).c_str();
	}

	json["simulatorType"] = simulatorType._to_string();

	if (action.has_value())
	{
		json["action"] = action.value()._to_string();
	}

	if (keyCode.has_value())
	{
		json["keyCode"] = keyCode.value();
	}

	if (!keyCodeModifiers.empty())
	{
		json["keyCodeModifiers"] = keyCodeModifiers;
	}

	if (mouseAxes.has_value())
	{
		json["mouseAxes"] = mouseAxes.value().toJson();
	}

	if (mouseButton.has_value())
	{
		json["mouseButton"] = mouseButton.value()._to_string();
	}

	if (xinputAxes.has_value())
	{
		json["xinputAxes"] = xinputAxes.value().toJson();
	}
}
