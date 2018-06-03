#include "stdafx.h"
#include "InputMap.h"

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

	return IsActive() ? PressedState::On : State;
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
{
	InputType         = other.InputType;
	InputButtons      = other.InputButtons;
	InputAxis         = other.InputAxis;
	InputRegion       = other.InputRegion;
	Toggle            = other.Toggle;
	RapidFire         = other.RapidFire;
	RapidFireInterval = other.RapidFireInterval;

	if (!other.InputAxisOptions.empty())
	{
		for (auto& pair : other.InputAxisOptions)
		{
			InputAxisOptions[pair.first] = pair.second;
		}
	}
}

InputMapBase::InputMapBase(::InputType_t inputType)
{
	InputType = inputType;
}

InputMapBase::InputMapBase(::InputType_t inputType, Ds4Buttons::T input)
{
	InputType    = inputType;
	InputButtons = input;
}

InputMapBase::InputMapBase(::InputType_t inputType, Ds4Axis::T input)
{
	InputType = inputType;
	InputAxis = input;
}

InputMapBase::InputMapBase(::InputType_t inputType, const std::string& input)
{
	InputType   = inputType;
	InputRegion = input;
}

void InputMapBase::Press()
{
	if (Toggle == true && State == PressedState::Pressed)
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

::InputAxisOptions InputMapBase::GetAxisOptions(Ds4Axis_t axis)
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
	return InputType == other.InputType
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

InputModifier::InputModifier(::InputType_t type, Ds4Buttons::T buttons)
	: InputMapBase(type, buttons)
{
}

InputModifier::InputModifier(::InputType_t type, Ds4Axis::T axis)
	: InputMapBase(type, axis)
{
}

InputModifier::InputModifier(::InputType_t type, const std::string& region)
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

InputMap::InputMap(const InputMap& other)
	: InputMapBase(other)
{
	SimulatorType    = other.SimulatorType;
	OutputType       = other.OutputType;
	Action           = other.Action;
	TouchDirection   = other.TouchDirection;
	//KeyCode        = other.KeyCode;
	MouseAxes        = other.MouseAxes;
	//MouseButton    = other.MouseButton;
	XInputButtons    = other.XInputButtons;
	XInputAxes       = other.XInputAxes;

	//KeyCodeModifiers = other.KeyCodeModifiers;
}

InputMap::InputMap(::SimulatorType simulatorType, InputType_t inputType, OutputType::T outputType)
	: InputMapBase(inputType)
{
	SimulatorType = simulatorType;
	OutputType    = outputType;
}

void InputMap::Press(const InputModifier& modifier)
{
	if (modifier.IsActive() != false)
	{
		InputMapBase::Press();
	}
	else if (Toggle == true && RapidFire == true)
	{
		UpdateRapidState();
	}
}

bool InputMap::operator==(const InputMap& other) const
{
	return SimulatorType == other.SimulatorType
	       && OutputType == other.OutputType
	       && Action == other.Action
	       // TODO: && KeyCode == other.KeyCode
	       // TODO: && KeyCodeModifiers == other.KeyCodeModifiers
	       && XInputButtons == other.XInputButtons
	       && XInputAxes == other.XInputAxes
	       && MouseAxes == other.MouseAxes
	       && TouchDirection == other.TouchDirection
	       /* TODO: && MouseButton == other.MouseButton*/;
}

bool InputMap::operator!=(const InputMap& other) const
{
	return !(*this == other);
}
