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

void InputMapBase::readJson(const QJsonObject& json)
{
	if (json.contains("inputType"))
	{
		InputType_t inputType_;
		ENUM_DESERIALIZE_FLAGS(InputType)(json["inputType"].toString().toStdString(), inputType_);
		inputType = inputType_;
	}

	if (json.contains("inputButtons"))
	{
		Ds4Buttons_t inputButtons_;
		ENUM_DESERIALIZE_FLAGS(Ds4Buttons)(json["inputButtons"].toString().toStdString(), inputButtons_);
		inputButtons = inputButtons_;
	}

	if (json.contains("inputAxis"))
	{
		Ds4Axis_t inputAxis_;
		ENUM_DESERIALIZE_FLAGS(Ds4Axis)(json["inputAxis"].toString().toStdString(), inputAxis_);
		inputAxis = inputAxis_;
	}

	inputRegion = json["inputRegion"].toString().toStdString();

	if (json.contains("toggle"))
	{
		toggle = json["toggle"].toBool();
	}

	if (json.contains("rapidFire"))
	{
		rapidFire = json["rapidFire"].toBool();
	}

	if (json.contains("rapidFireInterval"))
	{
		rapidFireInterval = std::chrono::nanoseconds(json["rapidFireInterval"].toInt());
	}

	QJsonObject inputAxisOptions_ = json["inputAxisOptions"].toObject();

	for (const auto& key : inputAxisOptions_.keys())
	{
		Ds4Axis_t flags;
		ENUM_DESERIALIZE_FLAGS(Ds4Axis)(key.toStdString(), flags);
		inputAxisOptions[flags] = fromJson<InputAxisOptions>(inputAxisOptions_[key].toObject());
	}
}

void InputMapBase::writeJson(QJsonObject& json) const
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

	QJsonObject inputAxisOptions_;

	for (const auto& pair : inputAxisOptions)
	{
		auto key = ENUM_SERIALIZE_FLAGS(Ds4Axis)(pair.first);
		inputAxisOptions_[key.c_str()] = pair.second.toJson();
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
	return bindings == other.bindings;
}

bool InputModifier::operator!=(const InputModifier& other) const
{
	return !(*this == other);
}

void InputModifier::readJson(const QJsonObject& json)
{
	InputMapBase::readJson(json);

	auto bindings_ = json["bindings"].toArray();

	for (const auto& value : bindings_)
	{
		bindings.push_back(fromJson<InputMap>(value.toObject()));
	}
}

void InputModifier::writeJson(QJsonObject& json) const
{
	InputMapBase::writeJson(json);
	
	QJsonArray bindings_;

	for (const auto& binding : bindings)
	{
		bindings_.append(binding.toJson());
	}

	json["bindings"] = bindings_;
}

InputMap::InputMap(const InputMap& other)
	: InputMapBase(other),
	  simulatorType(other.simulatorType),
	  outputType(other.outputType),
	  action(other.action),
	  touchDirection(other.touchDirection),
	  //KeyCode(other.KeyCode),
	  mouseAxes(other.mouseAxes),
	  //MouseButton(other.MouseButton),
	  xinputButtons(other.xinputButtons),
	  xinputAxes(other.xinputAxes)
	  //KeyCodeModifiers (other.KeyCodeModifiers),
{
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
	return simulatorType == other.simulatorType
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

void InputMap::readJson(const QJsonObject& json)
{
	InputMapBase::readJson(json);
	
	ENUM_DESERIALIZE_FLAGS(OutputType)(json["outputType"].toString().toStdString(), outputType);

	if (json.contains("xinputButtons"))
	{
		XInputButtons_t xinputButtons_;
		ENUM_DESERIALIZE_FLAGS(XInputButtons)(json["xinputButtons"].toString().toStdString(), xinputButtons_);
		xinputButtons = xinputButtons_;
	}

	if (json.contains("touchDirection"))
	{
		Direction_t touchDirection_;
		ENUM_DESERIALIZE_FLAGS(Direction)(json["touchDirection"].toString().toStdString(), touchDirection_);
		touchDirection = touchDirection_;
	}

	if (json.contains("simulatorType"))
	{
		simulatorType = SimulatorType::_from_string(json["simulatorType"].toString("none").toStdString().c_str());
	}

	if (json.contains("action"))
	{
		action = ActionType::_from_string(json["action"].toString("none").toStdString().c_str());
	}

	if (json.contains("mouseAxes"))
	{
		mouseAxes = fromJson<MouseAxes>(json["mouseAxes"].toObject());
	}

	if (json.contains("xinputAxes"))
	{
		xinputAxes = fromJson<XInputAxes>(json["xinputAxes"].toObject());
	}
}

void InputMap::writeJson(QJsonObject& json) const
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
