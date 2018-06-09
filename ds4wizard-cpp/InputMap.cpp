#include "stdafx.h"
#include "InputMap.h"

bool InputMapBase::PerformRapidFire() const
{
	return rapidStopwatch.elapsed() >= RapidFireInterval;
}

PressedState InputMapBase::SimulatedState() const
{
	if (RapidFire == true)
	{
		return rapidState;
	}

	if (Toggle != true)
	{
		return State;
	}

	if (IsActiveState(State))
	{
		return State;
	}

	return IsActive() ? PressedState::on : State;
}

bool InputMapBase::IsActive() const
{
	{
		if (Toggle == true)
		{
			return IsToggled;
		}

		return Pressable::IsActive();
	}
}

bool InputMapBase::IsPersistent() const
{
	return RapidFire == true;
}

InputMapBase::InputMapBase(const InputMapBase& other)
	: Pressable(other)
{
	inputType         = other.inputType;
	InputButtons      = other.InputButtons;
	InputAxis         = other.InputAxis;
	InputRegion       = other.InputRegion;
	Toggle            = other.Toggle;
	RapidFire         = other.RapidFire;
	RapidFireInterval = other.RapidFireInterval;
	InputAxisOptions  = other.InputAxisOptions;
}

InputMapBase::InputMapBase(InputType_t inputType)
{
	this->inputType = inputType;
}

InputMapBase::InputMapBase(InputType_t inputType, Ds4Buttons::T input)
{
	this->inputType    = inputType;
	InputButtons = input;
}

InputMapBase::InputMapBase(InputType_t inputType, Ds4Axis::T input)
{
	this->inputType = inputType;
	InputAxis = input;
}

InputMapBase::InputMapBase(InputType_t inputType, const std::string& input)
{
	this->inputType   = inputType;
	InputRegion = input;
}

void InputMapBase::Press()
{
	if (Toggle == true && State == PressedState::pressed)
	{
		IsToggled = !IsToggled;
	}

	Pressable::Press();

	if (RapidFire == true)
	{
		UpdateRapidState();
	}
}

void InputMapBase::UpdateRapidState()
{
	if (IsActive())
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
		if (PerformRapidFire())
		{
			rapidFiring = false;
			Pressable::Release(rapidState);
			rapidStopwatch.start();
		}
		else
		{
			Pressable::Press(rapidState);
		}
	}
	else
	{
		if (PerformRapidFire())
		{
			rapidFiring = true;
			Pressable::Press(rapidState);
			rapidStopwatch.start();
		}
		else
		{
			Pressable::Release(rapidState);
		}
	}
}

void InputMapBase::Release()
{
	Pressable::Release();

	if (RapidFire == true)
	{
		UpdateRapidState();
	}
}

InputAxisOptions InputMapBase::GetAxisOptions(Ds4Axis_t axis)
{
	if (InputAxisOptions.empty())
	{
		// HACK: remove
		return ::InputAxisOptions();
	}

	const auto it = InputAxisOptions.find(axis);

	if (it != InputAxisOptions.end())
	{
		return it->second;
	}

	// HACK: remove
	return ::InputAxisOptions();
}

bool InputMapBase::operator==(const InputMapBase& other) const
{
	return inputType == other.inputType
	       && InputButtons == other.InputButtons
	       && InputAxis == other.InputAxis
	       && InputRegion == other.InputRegion
	       && Toggle == other.Toggle
	       && RapidFire == other.RapidFire
	       && RapidFireInterval == other.RapidFireInterval
	       && InputAxisOptions == other.InputAxisOptions;
}

bool InputMapBase::operator!=(const InputMapBase& other) const
{
	return !(*this == other);
}

void InputMapBase::readJson(const QJsonObject& json)
{
	ENUM_DESERIALIZE_FLAGS(InputType)(json["inputType"].toString().toStdString(), inputType);
	ENUM_DESERIALIZE_FLAGS(Ds4Buttons)(json["inputButtons"].toString().toStdString(), InputButtons);
	ENUM_DESERIALIZE_FLAGS(Ds4Axis)(json["inputAxis"].toString().toStdString(), InputAxis);

	InputRegion       = json["inputRegion"].toString().toStdString();
	Toggle            = json["toggle"].toBool();
	RapidFire         = json["rapidFire"].toBool();
	RapidFireInterval = std::chrono::nanoseconds(json["rapidFireInterval"].toInt());

	QJsonObject inputAxisOptions_ = json["inputAxisOptions"].toObject();

	for (const auto& key : inputAxisOptions_.keys())
	{
		Ds4Axis_t flags;
		ENUM_DESERIALIZE_FLAGS(Ds4Axis)(key.toStdString(), flags);
		InputAxisOptions[flags] = fromJson<::InputAxisOptions>(inputAxisOptions_[key].toObject());
	}
}

void InputMapBase::writeJson(QJsonObject& json) const
{
	json["inputType"]         = ENUM_SERIALIZE_FLAGS(InputType)(inputType).c_str();
	json["inputButtons"]      = ENUM_SERIALIZE_FLAGS(Ds4Buttons)(InputButtons).c_str();
	json["inputAxis"]         = ENUM_SERIALIZE_FLAGS(Ds4Axis)(InputAxis).c_str();
	json["inputRegion"]       = InputRegion.c_str();
	json["toggle"]            = Toggle;
	json["rapidFire"]         = RapidFire;
	json["rapidFireInterval"] = RapidFireInterval.count();

	QJsonObject inputAxisOptions_;

	for (const auto& pair : InputAxisOptions)
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
	if (other.Bindings.empty())
	{
		return;
	}

	Bindings = other.Bindings;
}

bool InputModifier::operator==(const InputModifier& other) const
{
	return Bindings == other.Bindings;
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
		Bindings.push_back(fromJson<InputMap>(value.toObject()));
	}
}

void InputModifier::writeJson(QJsonObject& json) const
{
	InputMapBase::writeJson(json);
	
	QJsonArray bindings_;

	for (const auto& binding : Bindings)
	{
		bindings_.append(binding.toJson());
	}

	json["bindings"] = bindings_;
}

InputMap::InputMap(const InputMap& other)
	: InputMapBase(other)
{
	simulatorType    = other.simulatorType;
	outputType       = other.outputType;
	action           = other.action;
	touchDirection   = other.touchDirection;
	//KeyCode        = other.KeyCode;
	mouseAxes        = other.mouseAxes;
	//MouseButton    = other.MouseButton;
	xinputButtons    = other.xinputButtons;
	xinputAxes       = other.xinputAxes;

	//KeyCodeModifiers = other.KeyCodeModifiers;
}

InputMap::InputMap(SimulatorType simulatorType, InputType_t inputType, OutputType::T outputType)
	: InputMapBase(inputType)
{
	this->simulatorType = simulatorType;
	this->outputType    = outputType;
}

void InputMap::pressModifier(const InputModifier* modifier)
{
	if (!modifier || modifier->IsActive())
	{
		InputMapBase::Press();
	}
	else if (Toggle && RapidFire)
	{
		UpdateRapidState();
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
	ENUM_DESERIALIZE_FLAGS(XInputButtons)(json["xinputButtons"].toString().toStdString(), xinputButtons);
	ENUM_DESERIALIZE_FLAGS(Direction)(json["touchDirection"].toString().toStdString(), touchDirection);

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

	json["outputType"]     = ENUM_SERIALIZE_FLAGS(OutputType)(outputType).c_str();
	json["xinputButtons"]  = ENUM_SERIALIZE_FLAGS(XInputButtons)(xinputButtons).c_str();
	json["touchDirection"] = ENUM_SERIALIZE_FLAGS(Direction)(touchDirection).c_str();
	json["simulatorType"]  = simulatorType._to_string();
	json["action"]         = action._to_string();
	json["mouseAxes"]      = mouseAxes.toJson();
	json["xinputAxes"]     = xinputAxes.toJson();
}
