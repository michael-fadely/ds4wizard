#include "stdafx.h"
#include "pathutil.h"
#include "DeviceProfile.h"
#include "JsonData.h"

using namespace std::chrono;

std::string DeviceProfile::fileName() const
{
	return validatedFileName(name + ".json");
}

bool DeviceProfile::operator==(const DeviceProfile& other) const
{
	return DeviceSettingsCommon::operator==(other)
	       && name == other.name
	       && exclusiveMode == other.exclusiveMode
	       && useXInput == other.useXInput
	       && autoXInputIndex == other.autoXInputIndex
	       && xinputIndex == other.xinputIndex
	       && touchRegions == other.touchRegions
	       && bindings == other.bindings
	       && modifiers == other.modifiers;
}

void DeviceProfile::readJson(const nlohmann::json& json)
{
	DeviceSettingsCommon::readJson(json);

	name            = json["name"];
	exclusiveMode   = json["exclusiveMode"];
	useXInput       = json["useXInput"];
	autoXInputIndex = json.value("autoXInputIndex", true);
	xinputIndex     = json["xinputIndex"];

	auto touchRegions_ = json["touchRegions"].items();

	for (const auto& pair : touchRegions_)
	{
		// TODO: automatic pls
		touchRegions[pair.key()] = fromJson<Ds4TouchRegion>(pair.value());
	}

	for (const auto& value : json["bindings"])
	{
		bindings.push_back(fromJson<InputMap>(value));
	}

	for (const auto& value : json["modifiers"])
	{
		modifiers.push_back(fromJson<InputModifier>(value));
	}
}

void DeviceProfile::writeJson(nlohmann::json& json) const
{
	DeviceSettingsCommon::writeJson(json);

	json["name"]            = name.c_str();
	json["exclusiveMode"]   = exclusiveMode;
	json["useXInput"]       = useXInput;
	json["autoXInputIndex"] = autoXInputIndex;
	json["xinputIndex"]     = xinputIndex;

	nlohmann::json touchRegions_;

	for (auto& pair : touchRegions)
	{
		touchRegions_[pair.first.c_str()] = pair.second.toJson();
	}

	json["touchRegions"] = touchRegions_;

	nlohmann::json bindings_;

	for (auto& binding : modifiers)
	{
		bindings_.push_back(binding.toJson());
	}

	json["bindings"] = bindings_;

	nlohmann::json modifiers_;

	for (auto& modifier : modifiers)
	{
		modifiers_.push_back(modifier.toJson());
	}

	json["modifiers"] = modifiers_;
}

#pragma region trash

static constexpr auto DEFAULT_PROFILE_JSON = 
R"(
{
  "exclusiveMode": true,
  "useXInput": true,
  "autoXInputIndex": true,
  "xinputIndex": 0,
  "touchRegions": {
    "Left Half": {
      "type": "button",
      "allowCrossOver": false,
      "left": 0,
      "top": 0,
      "right": 959,
      "bottom": 941
    },
    "Right Half": {
      "type": "stickAutoCenter",
      "allowCrossOver": false,
      "left": 960,
      "top": 0,
      "right": 1919,
      "bottom": 941,
      "touchAxisOptions": {
        "up": {
          "deadZone": 0.25
        },
        "down": {
          "deadZone": 0.25
        },
        "left": {
          "deadZone": 0.25
        },
        "right": {
          "deadZone": 0.25
        }
      }
    }
  },
  "bindings": [
    {
      "simulatorType": "input",
      "outputType": "xinput",
      "touchDirection": "up",
      "xinputButtons": "dPadUp",
      "inputType": "touchRegion",
      "inputRegion": "Right Half"
    },
    {
      "simulatorType": "input",
      "outputType": "xinput",
      "touchDirection": "down",
      "xinputButtons": "dPadDown",
      "inputType": "touchRegion",
      "inputRegion": "Right Half"
    },
    {
      "simulatorType": "input",
      "outputType": "xinput",
      "touchDirection": "left",
      "xinputButtons": "dPadLeft",
      "inputType": "touchRegion",
      "inputRegion": "Right Half"
    },
    {
      "simulatorType": "input",
      "outputType": "xinput",
      "touchDirection": "right",
      "xinputButtons": "dPadRight",
      "inputType": "touchRegion",
      "inputRegion": "Right Half"
    },
    {
      "simulatorType": "input",
      "outputType": "xinput",
      "xinputButtons": "a",
      "inputType": "button",
      "inputButtons": "cross"
    },
    {
      "simulatorType": "input",
      "outputType": "xinput",
      "xinputButtons": "b",
      "inputType": "button",
      "inputButtons": "circle"
    },
    {
      "simulatorType": "input",
      "outputType": "xinput",
      "xinputButtons": "x",
      "inputType": "button",
      "inputButtons": "square"
    },
    {
      "simulatorType": "input",
      "outputType": "xinput",
      "xinputButtons": "y",
      "inputType": "button",
      "inputButtons": "triangle"
    },
    {
      "simulatorType": "input",
      "outputType": "xinput",
      "xinputButtons": "leftShoulder",
      "inputType": "button",
      "inputButtons": "l1"
    },
    {
      "simulatorType": "input",
      "outputType": "xinput",
      "xinputButtons": "rightShoulder",
      "inputType": "button",
      "inputButtons": "r1"
    },
    {
      "simulatorType": "input",
      "outputType": "xinput",
      "xinputButtons": "leftThumb",
      "inputType": "button",
      "inputButtons": "l3"
    },
    {
      "simulatorType": "input",
      "outputType": "xinput",
      "xinputButtons": "rightThumb",
      "inputType": "button",
      "inputButtons": "r3"
    },
    {
      "simulatorType": "input",
      "outputType": "xinput",
      "xinputButtons": "start",
      "inputType": "button",
      "inputButtons": "options"
    },
    {
      "simulatorType": "input",
      "outputType": "xinput",
      "xinputButtons": "back",
      "inputType": "button",
      "inputButtons": "share"
    },
    {
      "simulatorType": "input",
      "outputType": "xinput",
      "xinputButtons": "dPadUp",
      "inputType": "button",
      "inputButtons": "up"
    },
    {
      "simulatorType": "input",
      "outputType": "xinput",
      "xinputButtons": "dPadDown",
      "inputType": "button",
      "inputButtons": "down"
    },
    {
      "simulatorType": "input",
      "outputType": "xinput",
      "xinputButtons": "dPadLeft",
      "inputType": "button",
      "inputButtons": "left"
    },
    {
      "simulatorType": "input",
      "outputType": "xinput",
      "xinputButtons": "dPadRight",
      "inputType": "button",
      "inputButtons": "right"
    },
    {
      "simulatorType": "input",
      "outputType": "xinput",
      "xinputButtons": "guide",
      "inputType": "button",
      "inputButtons": "ps"
    },
    {
      "simulatorType": "input",
      "outputType": "xinput",
      "xinputAxes": {
        "axes": "leftStickY",
        "options": {
          "leftStickY": {
            "polarity": "positive"
          }
        }
      },
      "inputType": "axis",
      "inputAxis": "leftStickY",
      "inputAxisOptions": {
        "leftStickY": {
          "polarity": "positive"
        }
      }
    },
    {
      "simulatorType": "input",
      "outputType": "xinput",
      "xinputAxes": {
        "axes": "leftStickY",
        "options": {
          "leftStickY": {
            "polarity": "negative"
          }
        }
      },
      "inputType": "axis",
      "inputAxis": "leftStickY",
      "inputAxisOptions": {
        "leftStickY": {
          "polarity": "negative"
        }
      }
    },
    {
      "simulatorType": "input",
      "outputType": "xinput",
      "xinputAxes": {
        "axes": "leftStickX",
        "options": {
          "leftStickX": {
            "polarity": "positive"
          }
        }
      },
      "inputType": "axis",
      "inputAxis": "leftStickX",
      "inputAxisOptions": {
        "leftStickX": {
          "polarity": "positive"
        }
      }
    },
    {
      "simulatorType": "input",
      "outputType": "xinput",
      "xinputAxes": {
        "axes": "leftStickX",
        "options": {
          "leftStickX": {
            "polarity": "negative"
          }
        }
      },
      "inputType": "axis",
      "inputAxis": "leftStickX",
      "inputAxisOptions": {
        "leftStickX": {
          "polarity": "negative"
        }
      }
    },
    {
      "simulatorType": "input",
      "outputType": "xinput",
      "xinputAxes": {
        "axes": "rightStickY",
        "options": {
          "rightStickY": {
            "polarity": "positive"
          }
        }
      },
      "inputType": "axis",
      "inputAxis": "rightStickY",
      "inputAxisOptions": {
        "rightStickY": {
          "polarity": "positive"
        }
      }
    },
    {
      "simulatorType": "input",
      "outputType": "xinput",
      "xinputAxes": {
        "axes": "rightStickY",
        "options": {
          "rightStickY": {
            "polarity": "negative"
          }
        }
      },
      "inputType": "axis",
      "inputAxis": "rightStickY",
      "inputAxisOptions": {
        "rightStickY": {
          "polarity": "negative"
        }
      }
    },
    {
      "simulatorType": "input",
      "outputType": "xinput",
      "xinputAxes": {
        "axes": "rightStickX",
        "options": {
          "rightStickX": {
            "polarity": "positive"
          }
        }
      },
      "inputType": "axis",
      "inputAxis": "rightStickX",
      "inputAxisOptions": {
        "rightStickX": {
          "polarity": "positive"
        }
      }
    },
    {
      "simulatorType": "input",
      "outputType": "xinput",
      "xinputAxes": {
        "axes": "rightStickX",
        "options": {
          "rightStickX": {
            "polarity": "negative"
          }
        }
      },
      "inputType": "axis",
      "inputAxis": "rightStickX",
      "inputAxisOptions": {
        "rightStickX": {
          "polarity": "negative"
        }
      }
    },
    {
      "simulatorType": "input",
      "outputType": "xinput",
      "xinputAxes": {
        "axes": "leftTrigger"
      },
      "inputType": "axis",
      "inputAxis": "leftTrigger"
    },
    {
      "simulatorType": "input",
      "outputType": "xinput",
      "xinputAxes": {
        "axes": "rightTrigger"
      },
      "inputType": "axis",
      "inputAxis": "rightTrigger"
    }
  ],
  "modifiers": [
    {
      "bindings": [
        {
          "simulatorType": "input",
          "outputType": "xinput",
          "xinputButtons": "a",
          "inputType": "button",
          "inputButtons": "cross",
          "rapidFire": true,
          "rapidFireInterval": 16666
        },
        {
          "simulatorType": "input",
          "outputType": "xinput",
          "xinputButtons": "b",
          "inputType": "button",
          "inputButtons": "circle",
          "rapidFire": true,
          "rapidFireInterval": 16666
        },
        {
          "simulatorType": "input",
          "outputType": "xinput",
          "xinputButtons": "x",
          "inputType": "button",
          "inputButtons": "square",
          "rapidFire": true,
          "rapidFireInterval": 16666
        },
        {
          "simulatorType": "input",
          "outputType": "xinput",
          "xinputButtons": "y",
          "inputType": "button",
          "inputButtons": "triangle",
          "rapidFire": true,
          "rapidFireInterval": 16666
        },
        {
          "simulatorType": "input",
          "outputType": "xinput",
          "xinputButtons": "start",
          "inputType": "button",
          "inputButtons": "options",
          "rapidFire": true,
          "rapidFireInterval": 16666
        },
        {
          "simulatorType": "input",
          "outputType": "xinput",
          "xinputButtons": "back",
          "inputType": "button",
          "inputButtons": "share",
          "rapidFire": true,
          "rapidFireInterval": 16666
        }
      ],
      "inputType": "touchRegion",
      "inputRegion": "Left Half"
    },
    {
      "bindings": [
        {
          "simulatorType": "input",
          "outputType": "xinput",
          "xinputButtons": "leftThumb",
          "inputType": "button",
          "inputButtons": "touchButton"
        }
      ],
      "inputType": "touchRegion",
      "inputRegion": "Right Half"
    },
    {
      "bindings": [
        {
          "simulatorType": "action",
          "outputType": "none",
          "action": "bluetoothDisconnect",
          "inputType": "button",
          "inputButtons": "options"
        }
      ],
      "inputType": "button",
      "inputButtons": "ps"
    }
  ],
  "light": {
    "automaticColor": true,
    "color": {
      "red": 0,
      "green": 0,
      "blue": 255
    },
    "idleFade": true
  },
  "idle": {
    "disconnect": true,
    "timeout": 300
  },
  "notifyFullyCharged": true,
  "notifyBatteryLow": 2,
  "name": "Default"
})";

#pragma endregion

DeviceProfile DeviceProfile::defaultProfile()
{
	const auto obj = nlohmann::json::parse(DEFAULT_PROFILE_JSON);
	auto result = fromJson<DeviceProfile>(obj);
	return result;
}
