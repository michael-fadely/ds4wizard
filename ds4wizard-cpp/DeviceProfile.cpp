#include "stdafx.h"
#include "DeviceProfile.h"

std::string DeviceProfile::FileName() const
{
	// TODO
	/*
	static const char invalidCharacters[] = {
		'\\',
		'/',
		':',
		'*',
		'?',
		'"',
		'<',
		'>',
		'|'
	};

	for (char c : invalidCharacters)
	{
		FileName = FileName.Replace(c, '_');
	}
	*/

	return Name + ".json";
}

DeviceProfile::DeviceProfile(const DeviceProfile& other)
	: DeviceSettingsCommon(other)
{
	Name            = other.Name;
	ExclusiveMode   = other.ExclusiveMode;
	UseXInput       = other.UseXInput;
	AutoXInputIndex = other.AutoXInputIndex;
	XInputIndex     = other.XInputIndex;
	Idle            = other.Idle;

	Bindings  = other.Bindings;
	Modifiers = other.Modifiers;

	if (!other.TouchRegions.empty())
	{
		for (auto& pair : other.TouchRegions)
		{
			TouchRegions[pair.first] = pair.second;
		}
	}
}

bool DeviceProfile::operator==(const DeviceProfile& other) const
{
	return DeviceSettingsCommon::operator==(other) &&
	       Name == other.Name // TODO: case insensitive
	       && ExclusiveMode == other.ExclusiveMode
	       && UseXInput == other.UseXInput
	       && AutoXInputIndex == other.AutoXInputIndex
	       && XInputIndex == other.XInputIndex
	       && TouchRegions == other.TouchRegions
	       && Bindings == other.Bindings
	       && Modifiers == other.Modifiers;
}

void DeviceProfile::readJson(const QJsonObject& json)
{
	DeviceSettingsCommon::readJson(json);

	Name            = json["name"].toString().toStdString();
	ExclusiveMode   = json["exclusiveMode"].toBool();
	UseXInput       = json["useXInput"].toBool();
	AutoXInputIndex = json["autoXInput"].toBool();
	XInputIndex     = json["xInputIndex"].toInt();

}

void DeviceProfile::writeJson(QJsonObject& json) const
{
	DeviceSettingsCommon::writeJson(json);

	json["name"]          = Name.c_str();
	json["exclusiveMode"] = ExclusiveMode;
	json["useXInput"]     = UseXInput;
	json["autoXInput"]    = AutoXInputIndex;
	json["xInputIndex"]   = XInputIndex;
}

DeviceProfile DeviceProfile::Default()
{
	DeviceProfile result;
	/*
	result.Light = Ds4LightOptions(new Ds4Color(Color.Blue));
	result.Idle = DeviceIdleOptions(DeviceIdleOptions::Default);

	result.TouchRegions = new std::unordered_map<std::string, Ds4TouchRegion>
	{
		{ "Left Half", new Ds4TouchRegion(Ds4TouchRegionType.Button, 0, 0, 959, 941) };
	{
		"Right Half";
		new Ds4TouchRegion(Ds4TouchRegionType.StickAutoCenter, 960, 0, 1919, 941)
		{
			TouchAxisOptions = new std::unordered_map<Direction, InputAxisOptions>
		{
			{ Direction.Up, new InputAxisOptions { DeadZone = 0.25f } };
		{ Direction.Down, new InputAxisOptions { DeadZone = 0.25f } };
		{ Direction.Left, new InputAxisOptions { DeadZone = 0.25f } };
		{ Direction.Right, new InputAxisOptions { DeadZone = 0.25f } }
		}
		}
	}
	};

	Modifiers = new std::list<InputModifier>
	{
		new InputModifier(InputType.TouchRegion, "Left Half")
	{
		Bindings = new std::list<InputMap>
	{
		new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
	{
		InputButtons = Ds4Buttons.Cross;
	RapidFire = true;
	RapidFireInterval = TimeSpan.FromMilliseconds(1000.0 / 60.0);
	XInputButtons = XInputButtons.A
	};
	new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
	{
		InputButtons = Ds4Buttons.Circle;
	RapidFire = true;
	RapidFireInterval = TimeSpan.FromMilliseconds(1000.0 / 60.0);
	XInputButtons = XInputButtons.B
	};
	new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
	{
		InputButtons = Ds4Buttons.Square;
	RapidFire = true;
	RapidFireInterval = TimeSpan.FromMilliseconds(1000.0 / 60.0);
	XInputButtons = XInputButtons.X
	};
	new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
	{
		InputButtons = Ds4Buttons.Triangle;
	RapidFire = true;
	RapidFireInterval = TimeSpan.FromMilliseconds(1000.0 / 60.0);
	XInputButtons = XInputButtons.Y
	};
	new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
	{
		InputButtons = Ds4Buttons.Options;
	RapidFire = true;
	RapidFireInterval = TimeSpan.FromMilliseconds(1000.0 / 60.0);
	XInputButtons = XInputButtons.Start
	};
	new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
	{
		InputButtons = Ds4Buttons.Share;
	RapidFire = true;
	RapidFireInterval = TimeSpan.FromMilliseconds(1000.0 / 60.0);
	XInputButtons = XInputButtons.Back
	}
	}
	};
	new InputModifier(InputType.TouchRegion, "Right Half")
	{
		Bindings = new std::list<InputMap>
	{
		new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
	{
		InputButtons = Ds4Buttons.TouchButton;
	XInputButtons = XInputButtons.LeftThumb
	}
	}
	};
	new InputModifier(InputType.Button, Ds4Buttons.PS)
	{
		Bindings = new std::list<InputMap>
	{
		new InputMap(SimulatorType.Action, InputType.Button, OutputType.None)
	{
		InputButtons = Ds4Buttons.Options;
	Action = ActionType.BluetoothDisconnect
	}
	}
	}
	};

	Bindings = new std::list<InputMap>
	{
		#region Touch Binds

		new InputMap(SimulatorType.Input, InputType.TouchRegion, OutputType.XInput)
	{
		InputRegion = "Right Half";
	TouchDirection = Direction.Up;
	XInputButtons = XInputButtons.DPadUp
	};
	new InputMap(SimulatorType.Input, InputType.TouchRegion, OutputType.XInput)
	{
		InputRegion = "Right Half";
	TouchDirection = Direction.Down;
	XInputButtons = XInputButtons.DPadDown
	};
	new InputMap(SimulatorType.Input, InputType.TouchRegion, OutputType.XInput)
	{
		InputRegion = "Right Half";
	TouchDirection = Direction.Left;
	XInputButtons = XInputButtons.DPadLeft
	};
	new InputMap(SimulatorType.Input, InputType.TouchRegion, OutputType.XInput)
	{
		InputRegion = "Right Half";
	TouchDirection = Direction.Right;
	XInputButtons = XInputButtons.DPadRight
	};

	#endregion

		#region Buttons

		new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
	{
		InputButtons = Ds4Buttons.Cross;
	XInputButtons = XInputButtons.A
	};
	new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
	{
		InputButtons = Ds4Buttons.Circle;
	XInputButtons = XInputButtons.B
	};
	new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
	{
		InputButtons = Ds4Buttons.Square;
	XInputButtons = XInputButtons.X
	};
	new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
	{
		InputButtons = Ds4Buttons.Triangle;
	XInputButtons = XInputButtons.Y
	};

	new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
	{
		InputButtons = Ds4Buttons.L1;
	XInputButtons = XInputButtons.LeftShoulder
	};
	new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
	{
		InputButtons = Ds4Buttons.R1;
	XInputButtons = XInputButtons.RightShoulder
	};
	new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
	{
		InputButtons = Ds4Buttons.L3;
	XInputButtons = XInputButtons.LeftThumb
	};
	new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
	{
		InputButtons = Ds4Buttons.R3;
	XInputButtons = XInputButtons.RightThumb
	};

	new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
	{
		InputButtons = Ds4Buttons.Options;
	XInputButtons = XInputButtons.Start
	};
	new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
	{
		InputButtons = Ds4Buttons.Share;
	XInputButtons = XInputButtons.Back
	};

	new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
	{
		InputButtons = Ds4Buttons.Up;
	XInputButtons = XInputButtons.DPadUp
	};
	new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
	{
		InputButtons = Ds4Buttons.Down;
	XInputButtons = XInputButtons.DPadDown
	};
	new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
	{
		InputButtons = Ds4Buttons.Left;
	XInputButtons = XInputButtons.DPadLeft
	};
	new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
	{
		InputButtons = Ds4Buttons.Right;
	XInputButtons = XInputButtons.DPadRight
	};

	new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
	{
		InputButtons = Ds4Buttons.PS;
	XInputButtons = XInputButtons.Guide
	};

	#endregion

		#region Axes

		new InputMap(SimulatorType.Input, InputType.Axis, OutputType.XInput)
	{
		InputAxis = Ds4Axis.LeftStickY;

	InputAxisOptions = new std::unordered_map<Ds4Axis, InputAxisOptions>
	{
		{ Ds4Axis.LeftStickY, new InputAxisOptions(AxisPolarity.Positive) }
	};

	XInputAxes = new XInputAxes
	{
		Axes = XInputAxis.LeftStickY;
	Options = new std::unordered_map<XInputAxis, AxisOptions>
	{
		{ XInputAxis.LeftStickY, new AxisOptions(AxisPolarity.Positive) }
	}
	}
	};
	new InputMap(SimulatorType.Input, InputType.Axis, OutputType.XInput)
	{
		InputAxis = Ds4Axis.LeftStickY;

	InputAxisOptions = new std::unordered_map<Ds4Axis, InputAxisOptions>
	{
		{ Ds4Axis.LeftStickY, new InputAxisOptions(AxisPolarity.Negative) }
	};

	XInputAxes = new XInputAxes
	{
		Axes = XInputAxis.LeftStickY;
	Options = new std::unordered_map<XInputAxis, AxisOptions>
	{
		{ XInputAxis.LeftStickY, new AxisOptions(AxisPolarity.Negative) }
	}
	}
	};
	new InputMap(SimulatorType.Input, InputType.Axis, OutputType.XInput)
	{
		InputAxis = Ds4Axis.LeftStickX;

	InputAxisOptions = new std::unordered_map<Ds4Axis, InputAxisOptions>
	{
		{ Ds4Axis.LeftStickX, new InputAxisOptions(AxisPolarity.Positive) }
	};

	XInputAxes = new XInputAxes
	{
		Axes = XInputAxis.LeftStickX;
	Options = new std::unordered_map<XInputAxis, AxisOptions>
	{
		{ XInputAxis.LeftStickX, new AxisOptions(AxisPolarity.Positive) }
	}
	}
	};
	new InputMap(SimulatorType.Input, InputType.Axis, OutputType.XInput)
	{
		InputAxis = Ds4Axis.LeftStickX;

	InputAxisOptions = new std::unordered_map<Ds4Axis, InputAxisOptions>
	{
		{ Ds4Axis.LeftStickX, new InputAxisOptions(AxisPolarity.Negative) }
	};

	XInputAxes = new XInputAxes
	{
		Axes = XInputAxis.LeftStickX;
	Options = new std::unordered_map<XInputAxis, AxisOptions>
	{
		{ XInputAxis.LeftStickX, new AxisOptions(AxisPolarity.Negative) }
	}
	}
	};

	new InputMap(SimulatorType.Input, InputType.Axis, OutputType.XInput)
	{
		InputAxis = Ds4Axis.RightStickY;

	InputAxisOptions = new std::unordered_map<Ds4Axis, InputAxisOptions>
	{
		{ Ds4Axis.RightStickY, new InputAxisOptions(AxisPolarity.Positive) }
	};

	XInputAxes = new XInputAxes
	{
		Axes = XInputAxis.RightStickY;
	Options = new std::unordered_map<XInputAxis, AxisOptions>
	{
		{ XInputAxis.RightStickY, new AxisOptions(AxisPolarity.Positive) }
	}
	}
	};
	new InputMap(SimulatorType.Input, InputType.Axis, OutputType.XInput)
	{
		InputAxis = Ds4Axis.RightStickY;

	InputAxisOptions = new std::unordered_map<Ds4Axis, InputAxisOptions>
	{
		{ Ds4Axis.RightStickY, new InputAxisOptions(AxisPolarity.Negative) }
	};

	XInputAxes = new XInputAxes
	{
		Axes = XInputAxis.RightStickY;
	Options = new std::unordered_map<XInputAxis, AxisOptions>
	{
		{ XInputAxis.RightStickY, new AxisOptions(AxisPolarity.Negative) }
	}
	}
	};
	new InputMap(SimulatorType.Input, InputType.Axis, OutputType.XInput)
	{
		InputAxis = Ds4Axis.RightStickX;

	InputAxisOptions = new std::unordered_map<Ds4Axis, InputAxisOptions>
	{
		{ Ds4Axis.RightStickX, new InputAxisOptions(AxisPolarity.Positive) }
	};

	XInputAxes = new XInputAxes
	{
		Axes = XInputAxis.RightStickX;
	Options = new std::unordered_map<XInputAxis, AxisOptions>
	{
		{ XInputAxis.RightStickX, new AxisOptions(AxisPolarity.Positive) }
	}
	}
	};
	new InputMap(SimulatorType.Input, InputType.Axis, OutputType.XInput)
	{
		InputAxis = Ds4Axis.RightStickX;

	InputAxisOptions = new std::unordered_map<Ds4Axis, InputAxisOptions>
	{
		{ Ds4Axis.RightStickX, new InputAxisOptions(AxisPolarity.Negative) }
	};

	XInputAxes = new XInputAxes
	{
		Axes = XInputAxis.RightStickX;
	Options = new std::unordered_map<XInputAxis, AxisOptions>
	{
		{ XInputAxis.RightStickX, new AxisOptions(AxisPolarity.Negative) }
	}
	}
	};

	new InputMap(SimulatorType.Input, InputType.Axis, OutputType.XInput)
	{
		InputAxis = Ds4Axis.LeftTrigger;
	XInputAxes = new XInputAxes
	{
		Axes = XInputAxis.LeftTrigger
	}
	};
	new InputMap(SimulatorType.Input, InputType.Axis, OutputType.XInput)
	{
		InputAxis = Ds4Axis.RightTrigger;
	XInputAxes = new XInputAxes
	{
		Axes = XInputAxis.RightTrigger
	}
	}

		#endregion
	}
	*/

	// TODO

	return result;
}
