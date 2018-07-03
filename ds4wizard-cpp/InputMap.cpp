#include "stdafx.h"
#include "InputMap.h"

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
	  inputAxis(other.inputAxis),
	  inputRegion(other.inputRegion),
	  toggle(other.toggle),
	  rapidFire(other.rapidFire),
	  rapidFireInterval(other.rapidFireInterval),
	  inputAxisOptions(other.inputAxisOptions)
{
}

InputMapBase::InputMapBase(InputType_t inputType)
{
	this->inputType = inputType;
}

InputMapBase::InputMapBase(InputType_t inputType, Ds4Buttons::T input)
{
	this->inputType = inputType;
	inputButtons = input;
}

InputMapBase::InputMapBase(InputType_t inputType, Ds4Axis::T input)
{
	this->inputType = inputType;
	inputAxis = input;
}

InputMapBase::InputMapBase(InputType_t inputType, const std::string& input)
{
	this->inputType = inputType;
	inputRegion = input;
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

InputAxisOptions InputMapBase::getAxisOptions(Ds4Axis_t axis)
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
	       && inputAxis == other.inputAxis
	       && inputRegion == other.inputRegion
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

	if (json.find("inputAxis") != json.end())
	{
		Ds4Axis_t inputAxis_;
		ENUM_DESERIALIZE_FLAGS(Ds4Axis)(json["inputAxis"].get<std::string>(), inputAxis_);
		inputAxis = inputAxis_;
	}

	if (json.find("inputRegion") != json.end())
	{
		inputRegion = json["inputRegion"];
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
		rapidFireInterval = std::chrono::nanoseconds(json["rapidFireInterval"].get<int64_t>());
	}

	if (json.find("inputAxisOptions") != json.end())
	{
		auto inputAxisOptions_ = json["inputAxisOptions"].items();

		for (const auto& pair : inputAxisOptions_)
		{
			Ds4Axis_t flags;
			ENUM_DESERIALIZE_FLAGS(Ds4Axis)(pair.key(), flags);
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

	if (inputAxis.has_value())
	{
		json["inputAxis"] = ENUM_SERIALIZE_FLAGS(Ds4Axis)(inputAxis.value()).c_str();
	}

	json["inputRegion"] = inputRegion.c_str();

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
		auto key = ENUM_SERIALIZE_FLAGS(Ds4Axis)(pair.first);
		inputAxisOptions_[key] = pair.second.toJson();
	}

	json["inputAxisOptions"] = inputAxisOptions_;
}

InputModifier::InputModifier(InputType_t type, Ds4Buttons::T buttons)
	: InputMapBase(type, buttons)
{
}

InputModifier::InputModifier(InputType_t type, Ds4Axis::T axis)
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

InputMap::InputMap(SimulatorType simulatorType, InputType_t inputType, OutputType::T outputType)
	: InputMapBase(inputType)
{
	this->simulatorType = simulatorType;
	this->outputType    = outputType;
}

void InputMap::pressModifier(const InputModifier* modifier)
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
	       // TODO: && KeyCode == other.KeyCode
	       // TODO: && KeyCodeModifiers == other.KeyCodeModifiers
	       && xinputButtons == other.xinputButtons
	       && xinputAxes == other.xinputAxes
	       && mouseAxes == other.mouseAxes
	       && touchDirection == other.touchDirection
	       /* TODO: && MouseButton == other.MouseButton*/;
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

	if (json.find("mouseAxes") != json.end())
	{
		mouseAxes = fromJson<MouseAxes>(json["mouseAxes"]);
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

	json["mouseAxes"] = mouseAxes.toJson();

	if (xinputAxes.has_value())
	{
		json["xinputAxes"] = xinputAxes.value().toJson();
	}
}
