#include "stdafx.h"
#include "DeviceProfile.h"

using namespace std::chrono;

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
	XInputIndex     = json["xinputIndex"].toInt();

	auto touchRegions_ = json["touchRegions"].toObject();

	for (const auto& key : touchRegions_.keys())
	{
		auto stdstr = key.toStdString();
		TouchRegions[stdstr] = fromJson<Ds4TouchRegion>(touchRegions_[key].toObject());
	}

	auto bindings_ = json["bindings"].toArray();

	for (auto& value : bindings_)
	{
		Bindings.push_back(fromJson<InputMap>(value.toObject()));
	}

	auto modifiers_ = json["modifiers"].toArray();

	for (auto& value : modifiers_)
	{
		Modifiers.push_back(fromJson<InputModifier>(value.toObject()));
	}
}

void DeviceProfile::writeJson(QJsonObject& json) const
{
	DeviceSettingsCommon::writeJson(json);

	json["name"]          = Name.c_str();
	json["exclusiveMode"] = ExclusiveMode;
	json["useXInput"]     = UseXInput;
	json["autoXInput"]    = AutoXInputIndex;
	json["xinputIndex"]   = XInputIndex;

	QJsonObject touchRegions_;

	for (auto& pair : TouchRegions)
	{
		touchRegions_[pair.first.c_str()] = pair.second.toJson();
	}

	json["touchRegions"] = touchRegions_;

	QJsonArray bindings_;

	for (auto& binding : Modifiers)
	{
		bindings_.append(binding.toJson());
	}

	json["bindings"] = bindings_;

	QJsonArray modifiers_;

	for (auto& modifier : Modifiers)
	{
		modifiers_.append(modifier.toJson());
	}

	json["modifiers"] = modifiers_;
}

#pragma region trash
const char* a = "{" \
"  \"exclusiveMode\": true," \
"  \"useXInput\": true," \
"  \"autoXInputIndex\": true," \
"  \"xinputIndex\": 0," \
"  \"touchRegions\": {" \
"    \"Left Half\": {" \
"      \"type\": \"button\"," \
"      \"allowCrossOver\": false," \
"      \"left\": 0," \
"      \"top\": 0," \
"      \"right\": 959," \
"      \"bottom\": 941" \
"    }," \
"    \"Right Half\": {" \
"      \"type\": \"stickAutoCenter\"," \
"      \"allowCrossOver\": false," \
"      \"left\": 960," \
"      \"top\": 0," \
"      \"right\": 1919," \
"      \"bottom\": 941," \
"      \"touchAxisOptions\": {" \
"        \"up\": {" \
"          \"deadZone\": 0.25" \
"        }," \
"        \"down\": {" \
"          \"deadZone\": 0.25" \
"        }," \
"        \"left\": {" \
"          \"deadZone\": 0.25" \
"        }," \
"        \"right\": {" \
"          \"deadZone\": 0.25" \
"        }" \
"      }" \
"    }" \
"  }," \
"  \"bindings\": [" \
"    {" \
"      \"simulatorType\": \"input\"," \
"      \"outputType\": \"xinput\"," \
"      \"touchDirection\": \"up\"," \
"      \"xinputButtons\": \"dPadUp\"," \
"      \"inputType\": \"touchRegion\"," \
"      \"inputRegion\": \"Right Half\"" \
"    }," \
"    {" \
"      \"simulatorType\": \"input\"," \
"      \"outputType\": \"xinput\"," \
"      \"touchDirection\": \"down\"," \
"      \"xinputButtons\": \"dPadDown\"," \
"      \"inputType\": \"touchRegion\"," \
"      \"inputRegion\": \"Right Half\"" \
"    }," \
"    {" \
"      \"simulatorType\": \"input\"," \
"      \"outputType\": \"xinput\"," \
"      \"touchDirection\": \"left\"," \
"      \"xinputButtons\": \"dPadLeft\"," \
"      \"inputType\": \"touchRegion\"," \
"      \"inputRegion\": \"Right Half\"" \
"    }," \
"    {" \
"      \"simulatorType\": \"input\"," \
"      \"outputType\": \"xinput\"," \
"      \"touchDirection\": \"right\"," \
"      \"xinputButtons\": \"dPadRight\"," \
"      \"inputType\": \"touchRegion\"," \
"      \"inputRegion\": \"Right Half\"" \
"    }" \
"  ]," \
"  \"modifiers\": [" \
"    {" \
"      \"bindings\": [" \
"        {" \
"          \"simulatorType\": \"input\"," \
"          \"outputType\": \"xinput\"," \
"          \"xinputButtons\": \"a\"," \
"          \"inputType\": \"button\"," \
"          \"inputButtons\": \"cross\"," \
"          \"rapidFire\": true," \
"          \"rapidFireInterval\": 16666666" \
"        }," \
"        {" \
"          \"simulatorType\": \"input\"," \
"          \"outputType\": \"xinput\"," \
"          \"xinputButtons\": \"b\"," \
"          \"inputType\": \"button\"," \
"          \"inputButtons\": \"circle\"," \
"          \"rapidFire\": true," \
"          \"rapidFireInterval\": 16666666" \
"        }," \
"        {" \
"          \"simulatorType\": \"input\"," \
"          \"outputType\": \"xinput\"," \
"          \"xinputButtons\": \"x\"," \
"          \"inputType\": \"button\"," \
"          \"inputButtons\": \"square\"," \
"          \"rapidFire\": true," \
"          \"rapidFireInterval\": 16666666" \
"        }," \
"        {" \
"          \"simulatorType\": \"input\"," \
"          \"outputType\": \"xinput\"," \
"          \"xinputButtons\": \"y\"," \
"          \"inputType\": \"button\"," \
"          \"inputButtons\": \"triangle\"," \
"          \"rapidFire\": true," \
"          \"rapidFireInterval\": 16666666" \
"        }," \
"        {" \
"          \"simulatorType\": \"input\"," \
"          \"outputType\": \"xinput\"," \
"          \"xinputButtons\": \"start\"," \
"          \"inputType\": \"button\"," \
"          \"inputButtons\": \"options\"," \
"          \"rapidFire\": true," \
"          \"rapidFireInterval\": 16666666" \
"        }," \
"        {" \
"          \"simulatorType\": \"input\"," \
"          \"outputType\": \"xinput\"," \
"          \"xinputButtons\": \"back\"," \
"          \"inputType\": \"button\"," \
"          \"inputButtons\": \"share\"," \
"          \"rapidFire\": true," \
"          \"rapidFireInterval\": 16666666" \
"        }" \
"      ]," \
"      \"inputType\": \"touchRegion\"," \
"      \"inputRegion\": \"Left Half\"" \
"    }," \
"    {" \
"      \"bindings\": [" \
"        {" \
"          \"simulatorType\": \"input\"," \
"          \"outputType\": \"xinput\"," \
"          \"xinputButtons\": \"leftThumb\"," \
"          \"inputType\": \"button\"," \
"          \"inputButtons\": \"touchButton\"" \
"        }" \
"      ]," \
"      \"inputType\": \"touchRegion\"," \
"      \"inputRegion\": \"Right Half\"" \
"    }," \
"    {" \
"      \"bindings\": [" \
"        {" \
"          \"simulatorType\": \"action\"," \
"          \"outputType\": \"none\"," \
"          \"action\": \"bluetoothDisconnect\"," \
"          \"inputType\": \"button\"," \
"          \"inputButtons\": \"options\"" \
"        }" \
"      ]," \
"      \"inputType\": \"button\"," \
"      \"inputButtons\": \"ps\"" \
"    }" \
"  ]," \
"  \"light\": {" \
"    \"automaticColor\": true," \
"    \"color\": {" \
"      \"r\": 0," \
"      \"g\": 0," \
"      \"b\": 255" \
"    }," \
"    \"idleFade\": true" \
"  }," \
"  \"idle\": {" \
"    \"disconnect\": true," \
"    \"unit\": \"minutes\"," \
"    \"timeSpan\": 300000" \
"  }," \
"  \"notifyFullyCharged\": true," \
"  \"notifyBatteryLow\": 2," \
"  \"name\": \"wangis\"" \
"}";
#pragma endregion

DeviceProfile DeviceProfile::Default()
{
	auto obj = QJsonDocument::fromJson(QString(a).toUtf8());
	auto result = fromJson<DeviceProfile>(obj.object());

/*
	result.Light = Ds4LightOptions(Ds4Color(0, 0, 255));
	result.Idle  = DeviceIdleOptions(DeviceIdleOptions::Default);

	Ds4TouchRegion leftHalf(Ds4TouchRegionType::button, 0, 0, 959, 941);
	result.TouchRegions["Left Half"] = leftHalf;

	Ds4TouchRegion rightHalf(Ds4TouchRegionType::StickAutoCenter, 960, 0, 1919, 941);

	InputAxisOptions touchAxes[4] = {};

	for (auto& a : touchAxes)
	{
		a.DeadZone = 0.25f;
	}

	rightHalf.TouchAxisOptions[Direction::Up]    = touchAxes[0];
	rightHalf.TouchAxisOptions[Direction::Down]  = touchAxes[1];
	rightHalf.TouchAxisOptions[Direction::Left]  = touchAxes[2];
	rightHalf.TouchAxisOptions[Direction::Right] = touchAxes[3];

	result.TouchRegions["Right Half"] = rightHalf;

	InputModifier leftHalfModifier(InputType::touchRegion, "Left Half");

	const auto rapidFireInterval = duration_cast<high_resolution_clock::duration>(duration<double, std::milli>(1000.0 / 60.0));
	InputMap map;

	map                   = InputMap(SimulatorType::Input, InputType::button, OutputType::XInput);
	map.InputButtons      = Ds4Buttons::Cross;
	map.RapidFire         = true;
	map.RapidFireInterval = duration_cast<high_resolution_clock::duration>(duration<double, std::milli>(1000.0 / 60.0));
	map.XInputButtons     = XInputButtons::A;

	Modifiers.push_back(map);

	map                   = InputMap(SimulatorType::Input, InputType::button, OutputType::XInput);
	map.InputButtons      = Ds4Buttons::Circle;
	map.RapidFire         = true;
	map.RapidFireInterval = rapidFireInterval;
	map.XInputButtons     = XInputButtons::B;

	map                   = InputMap(SimulatorType::Input, InputType::button, OutputType::XInput);
	map.InputButtons      = Ds4Buttons::Square;
	map.RapidFire         = true;
	map.RapidFireInterval = rapidFireInterval;
	map.XInputButtons     = XInputButtons::X;

	InputMap(SimulatorType::Input, InputType::button, OutputType::XInput)
	{
		InputButtons = Ds4Buttons::Triangle;
		RapidFire = true;
		RapidFireInterval = TimeSpan.FromMilliseconds(1000.0 / 60.0);
		XInputButtons = XInputButtons::Y;
	};
	InputMap(SimulatorType::Input, InputType::button, OutputType::XInput)
	{
		InputButtons = Ds4Buttons::Options;
		RapidFire = true;
		RapidFireInterval = TimeSpan.FromMilliseconds(1000.0 / 60.0);
		XInputButtons = XInputButtons::Start;
	};
	InputMap(SimulatorType::Input, InputType::button, OutputType::XInput)
	{
		InputButtons = Ds4Buttons::Share;
		RapidFire = true;
		RapidFireInterval = TimeSpan.FromMilliseconds(1000.0 / 60.0);
		XInputButtons = XInputButtons::Back;
	}

	Modifiers = 
	{
		InputModifier()
		{
			Bindings =
			{
				
			}
		};
		InputModifier(InputType::touchRegion, "Right Half")
		{
			Bindings = std::list<InputMap>
			{
				InputMap(SimulatorType::Input, InputType::button, OutputType::XInput)
				{
					InputButtons  = Ds4Buttons::TouchButton;
					XInputButtons = XInputButtons::LeftThumb
				}
			}
		};
		InputModifier(InputType::button, Ds4Buttons::PS)
		{
			Bindings = std::list<InputMap>
			{
				InputMap(SimulatorType::Action, InputType::button, OutputType::None)
				{
					InputButtons = Ds4Buttons::Options;
					Action       = ActionType::BluetoothDisconnect
				}
			}
		}
	};

	Bindings = 
	{
		#pragma region Touch Binds

		InputMap(SimulatorType::Input, InputType::touchRegion, OutputType::XInput)
		{
			InputRegion    = "Right Half";
			TouchDirection = Direction::Up;
			XInputButtons  = XInputButtons::DPadUp
		};
		InputMap(SimulatorType::Input, InputType::touchRegion, OutputType::XInput)
		{
			InputRegion    = "Right Half";
			TouchDirection = Direction::Down;
			XInputButtons  = XInputButtons::DPadDown
		};
		InputMap(SimulatorType::Input, InputType::touchRegion, OutputType::XInput)
		{
			InputRegion    = "Right Half";
			TouchDirection = Direction::Left;
			XInputButtons  = XInputButtons::DPadLeft
		};
		InputMap(SimulatorType::Input, InputType::touchRegion, OutputType::XInput)
		{
			InputRegion    = "Right Half";
			TouchDirection = Direction::Right;
			XInputButtons  = XInputButtons::DPadRight
		};

	#pragma endregion

		#pragma region Buttons

		InputMap(SimulatorType::Input, InputType::button, OutputType::XInput)
		{
			InputButtons  = Ds4Buttons::Cross;
			XInputButtons = XInputButtons::A
		};
		InputMap(SimulatorType::Input, InputType::button, OutputType::XInput)
		{
			InputButtons  = Ds4Buttons::Circle;
			XInputButtons = XInputButtons::B
		};
		InputMap(SimulatorType::Input, InputType::button, OutputType::XInput)
		{
			InputButtons  = Ds4Buttons::Square;
			XInputButtons = XInputButtons::X
		};
		InputMap(SimulatorType::Input, InputType::button, OutputType::XInput)
		{
			InputButtons  = Ds4Buttons::Triangle;
			XInputButtons = XInputButtons::Y
		};

		InputMap(SimulatorType::Input, InputType::button, OutputType::XInput)
		{
			InputButtons  = Ds4Buttons::L1;
			XInputButtons = XInputButtons::LeftShoulder
		};
		InputMap(SimulatorType::Input, InputType::button, OutputType::XInput)
		{
			InputButtons  = Ds4Buttons::R1;
			XInputButtons = XInputButtons::RightShoulder
		};
		InputMap(SimulatorType::Input, InputType::button, OutputType::XInput)
		{
			InputButtons  = Ds4Buttons::L3;
			XInputButtons = XInputButtons::LeftThumb
		};
		InputMap(SimulatorType::Input, InputType::button, OutputType::XInput)
		{
			InputButtons  = Ds4Buttons::R3;
			XInputButtons = XInputButtons::RightThumb
		};

		InputMap(SimulatorType::Input, InputType::button, OutputType::XInput)
		{
			InputButtons  = Ds4Buttons::Options;
			XInputButtons = XInputButtons::Start
		};
		InputMap(SimulatorType::Input, InputType::button, OutputType::XInput)
		{
			InputButtons  = Ds4Buttons::Share;
			XInputButtons = XInputButtons::Back
		};

		InputMap(SimulatorType::Input, InputType::button, OutputType::XInput)
		{
			InputButtons  = Ds4Buttons::Up;
			XInputButtons = XInputButtons::DPadUp
		};
		InputMap(SimulatorType::Input, InputType::button, OutputType::XInput)
		{
			InputButtons  = Ds4Buttons::Down;
			XInputButtons = XInputButtons::DPadDown
		};
		InputMap(SimulatorType::Input, InputType::button, OutputType::XInput)
		{
			InputButtons  = Ds4Buttons::Left;
			XInputButtons = XInputButtons::DPadLeft
		};
		InputMap(SimulatorType::Input, InputType::button, OutputType::XInput)
		{
			InputButtons  = Ds4Buttons::Right;
			XInputButtons = XInputButtons::DPadRight
		};

		InputMap(SimulatorType::Input, InputType::button, OutputType::XInput)
		{
			InputButtons  = Ds4Buttons::PS;
			XInputButtons = XInputButtons::Guide
		};

	#pragma endregion

		#pragma region Axes

		InputMap(SimulatorType::Input, InputType::axis, OutputType::XInput)
		{
			InputAxis = Ds4Axis::leftStickY;

			InputAxisOptions = std::unordered_map<Ds4Axis, InputAxisOptions>
			{
				{ Ds4Axis::leftStickY, InputAxisOptions(AxisPolarity::positive) }
			};

			XInputAxes = XInputAxes
			{
				Axes    = XInputAxis::leftStickY;
				Options = std::unordered_map<XInputAxis, AxisOptions>
				{
					{ XInputAxis::leftStickY, AxisOptions(AxisPolarity::positive) }
				}
			}
		};
		InputMap(SimulatorType::Input, InputType::axis, OutputType::XInput)
		{
			InputAxis = Ds4Axis::leftStickY;

			InputAxisOptions = std::unordered_map<Ds4Axis, InputAxisOptions>
			{
				{ Ds4Axis::leftStickY, InputAxisOptions(AxisPolarity::negative) }
			};

			XInputAxes = XInputAxes
			{
				Axes    = XInputAxis::leftStickY;
				Options = std::unordered_map<XInputAxis, AxisOptions>
				{
					{ XInputAxis::leftStickY, AxisOptions(AxisPolarity::negative) }
				}
			}
		};
		InputMap(SimulatorType::Input, InputType::axis, OutputType::XInput)
		{
			InputAxis = Ds4Axis::leftStickX;

			InputAxisOptions = std::unordered_map<Ds4Axis, InputAxisOptions>
			{
				{ Ds4Axis::leftStickX, InputAxisOptions(AxisPolarity::positive) }
			};

			XInputAxes = XInputAxes
			{
				Axes    = XInputAxis::leftStickX;
				Options = std::unordered_map<XInputAxis, AxisOptions>
				{
					{ XInputAxis::leftStickX, AxisOptions(AxisPolarity::positive) }
				}
			}
		};
		InputMap(SimulatorType::Input, InputType::axis, OutputType::XInput)
		{
			InputAxis = Ds4Axis::leftStickX;

			InputAxisOptions = std::unordered_map<Ds4Axis, InputAxisOptions>
			{
				{ Ds4Axis::leftStickX, InputAxisOptions(AxisPolarity::negative) }
			};

			XInputAxes = XInputAxes
			{
				Axes    = XInputAxis::leftStickX;
				Options = std::unordered_map<XInputAxis, AxisOptions>
				{
					{ XInputAxis::leftStickX, AxisOptions(AxisPolarity::negative) }
				}
			}
		};

		InputMap(SimulatorType::Input, InputType::axis, OutputType::XInput)
		{
			InputAxis = Ds4Axis::rightStickY;

			InputAxisOptions = std::unordered_map<Ds4Axis, InputAxisOptions>
			{
				{ Ds4Axis::rightStickY, InputAxisOptions(AxisPolarity::positive) }
			};

			XInputAxes = XInputAxes
			{
				Axes    = XInputAxis::rightStickY;
				Options = std::unordered_map<XInputAxis, AxisOptions>
				{
					{ XInputAxis::rightStickY, AxisOptions(AxisPolarity::positive) }
				}
			}
		};
		InputMap(SimulatorType::Input, InputType::axis, OutputType::XInput)
		{
			InputAxis = Ds4Axis::rightStickY;

			InputAxisOptions = std::unordered_map<Ds4Axis, InputAxisOptions>
			{
				{ Ds4Axis::rightStickY, InputAxisOptions(AxisPolarity::negative) }
			};

			XInputAxes = XInputAxes
			{
				Axes    = XInputAxis::rightStickY;
				Options = std::unordered_map<XInputAxis, AxisOptions>
				{
					{ XInputAxis::rightStickY, AxisOptions(AxisPolarity::negative) }
				}
			}
		};
		InputMap(SimulatorType::Input, InputType::axis, OutputType::XInput)
		{
			InputAxis = Ds4Axis::rightStickX;

			InputAxisOptions = std::unordered_map<Ds4Axis, InputAxisOptions>
			{
				{ Ds4Axis::rightStickX, InputAxisOptions(AxisPolarity::positive) }
			};

			XInputAxes = XInputAxes
			{
				Axes    = XInputAxis::rightStickX;
				Options = std::unordered_map<XInputAxis, AxisOptions>
				{
					{ XInputAxis::rightStickX, AxisOptions(AxisPolarity::positive) }
				}
			}
		};
		InputMap(SimulatorType::Input, InputType::axis, OutputType::XInput)
		{
			InputAxis = Ds4Axis::rightStickX;

			InputAxisOptions = std::unordered_map<Ds4Axis, InputAxisOptions>
			{
				{ Ds4Axis::rightStickX, InputAxisOptions(AxisPolarity::negative) }
			};

			XInputAxes = XInputAxes
			{
				Axes    = XInputAxis::rightStickX;
				Options = std::unordered_map<XInputAxis, AxisOptions>
				{
					{ XInputAxis::rightStickX, AxisOptions(AxisPolarity::negative) }
				}
			}
		};

		InputMap(SimulatorType::Input, InputType::axis, OutputType::XInput)
		{
			InputAxis  = Ds4Axis::leftTrigger;
			XInputAxes = XInputAxes
			{
				Axes = XInputAxis::leftTrigger
			}
		};
		InputMap(SimulatorType::Input, InputType::axis, OutputType::XInput)
		{
			InputAxis  = Ds4Axis::rightTrigger;
			XInputAxes = XInputAxes
			{
				Axes = XInputAxis::rightTrigger
			}
		}

		#pragma endregion
	}*/

	return result;
}
