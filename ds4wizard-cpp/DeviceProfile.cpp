#include "stdafx.h"
#include "DeviceProfile.h"

using namespace std::chrono;

std::string DeviceProfile::fileName() const
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

	return name + ".json";
}

DeviceProfile::DeviceProfile(const DeviceProfile& other)
	: DeviceSettingsCommon(other)
{
	name            = other.name;
	exclusiveMode   = other.exclusiveMode;
	useXInput       = other.useXInput;
	autoXInputIndex = other.autoXInputIndex;
	xinputIndex     = other.xinputIndex;
	idle            = other.idle;

	bindings  = other.bindings;
	modifiers = other.modifiers;

	if (!other.touchRegions.empty())
	{
		for (auto& pair : other.touchRegions)
		{
			touchRegions[pair.first] = pair.second;
		}
	}
}

bool DeviceProfile::operator==(const DeviceProfile& other) const
{
	return DeviceSettingsCommon::operator==(other) &&
	       name == other.name // TODO: case insensitive
	       && exclusiveMode == other.exclusiveMode
	       && useXInput == other.useXInput
	       && autoXInputIndex == other.autoXInputIndex
	       && xinputIndex == other.xinputIndex
	       && touchRegions == other.touchRegions
	       && bindings == other.bindings
	       && modifiers == other.modifiers;
}

void DeviceProfile::readJson(const QJsonObject& json)
{
	DeviceSettingsCommon::readJson(json);

	name            = json["name"].toString().toStdString();
	exclusiveMode   = json["exclusiveMode"].toBool();
	useXInput       = json["useXInput"].toBool();
	autoXInputIndex = json["autoXInput"].toBool();
	xinputIndex     = json["xinputIndex"].toInt();

	auto touchRegions_ = json["touchRegions"].toObject();

	for (const auto& key : touchRegions_.keys())
	{
		auto stdstr = key.toStdString();
		touchRegions[stdstr] = fromJson<Ds4TouchRegion>(touchRegions_[key].toObject());
	}

	auto bindings_ = json["bindings"].toArray();

	for (auto& value : bindings_)
	{
		bindings.push_back(fromJson<InputMap>(value.toObject()));
	}

	auto modifiers_ = json["modifiers"].toArray();

	for (auto& value : modifiers_)
	{
		modifiers.push_back(fromJson<InputModifier>(value.toObject()));
	}
}

void DeviceProfile::writeJson(QJsonObject& json) const
{
	DeviceSettingsCommon::writeJson(json);

	json["name"]          = name.c_str();
	json["exclusiveMode"] = exclusiveMode;
	json["useXInput"]     = useXInput;
	json["autoXInput"]    = autoXInputIndex;
	json["xinputIndex"]   = xinputIndex;

	QJsonObject touchRegions_;

	for (auto& pair : touchRegions)
	{
		touchRegions_[pair.first.c_str()] = pair.second.toJson();
	}

	json["touchRegions"] = touchRegions_;

	QJsonArray bindings_;

	for (auto& binding : modifiers)
	{
		bindings_.append(binding.toJson());
	}

	json["bindings"] = bindings_;

	QJsonArray modifiers_;

	for (auto& modifier : modifiers)
	{
		modifiers_.append(modifier.toJson());
	}

	json["modifiers"] = modifiers_;
}

#pragma region trash

const char* defaultProfileJson = "{" \
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
"    }," \
"    {" \
"      \"simulatorType\": \"input\"," \
"      \"outputType\": \"xinput\"," \
"      \"xinputButtons\": \"a\"," \
"      \"inputType\": \"button\"," \
"      \"inputButtons\": \"cross\"" \
"    }," \
"    {" \
"      \"simulatorType\": \"input\"," \
"      \"outputType\": \"xinput\"," \
"      \"xinputButtons\": \"b\"," \
"      \"inputType\": \"button\"," \
"      \"inputButtons\": \"circle\"" \
"    }," \
"    {" \
"      \"simulatorType\": \"input\"," \
"      \"outputType\": \"xinput\"," \
"      \"xinputButtons\": \"x\"," \
"      \"inputType\": \"button\"," \
"      \"inputButtons\": \"square\"" \
"    }," \
"    {" \
"      \"simulatorType\": \"input\"," \
"      \"outputType\": \"xinput\"," \
"      \"xinputButtons\": \"y\"," \
"      \"inputType\": \"button\"," \
"      \"inputButtons\": \"triangle\"" \
"    }," \
"    {" \
"      \"simulatorType\": \"input\"," \
"      \"outputType\": \"xinput\"," \
"      \"xinputButtons\": \"leftShoulder\"," \
"      \"inputType\": \"button\"," \
"      \"inputButtons\": \"l1\"" \
"    }," \
"    {" \
"      \"simulatorType\": \"input\"," \
"      \"outputType\": \"xinput\"," \
"      \"xinputButtons\": \"rightShoulder\"," \
"      \"inputType\": \"button\"," \
"      \"inputButtons\": \"r1\"" \
"    }," \
"    {" \
"      \"simulatorType\": \"input\"," \
"      \"outputType\": \"xinput\"," \
"      \"xinputButtons\": \"leftThumb\"," \
"      \"inputType\": \"button\"," \
"      \"inputButtons\": \"l3\"" \
"    }," \
"    {" \
"      \"simulatorType\": \"input\"," \
"      \"outputType\": \"xinput\"," \
"      \"xinputButtons\": \"rightThumb\"," \
"      \"inputType\": \"button\"," \
"      \"inputButtons\": \"r3\"" \
"    }," \
"    {" \
"      \"simulatorType\": \"input\"," \
"      \"outputType\": \"xinput\"," \
"      \"xinputButtons\": \"start\"," \
"      \"inputType\": \"button\"," \
"      \"inputButtons\": \"options\"" \
"    }," \
"    {" \
"      \"simulatorType\": \"input\"," \
"      \"outputType\": \"xinput\"," \
"      \"xinputButtons\": \"back\"," \
"      \"inputType\": \"button\"," \
"      \"inputButtons\": \"share\"" \
"    }," \
"    {" \
"      \"simulatorType\": \"input\"," \
"      \"outputType\": \"xinput\"," \
"      \"xinputButtons\": \"dPadUp\"," \
"      \"inputType\": \"button\"," \
"      \"inputButtons\": \"up\"" \
"    }," \
"    {" \
"      \"simulatorType\": \"input\"," \
"      \"outputType\": \"xinput\"," \
"      \"xinputButtons\": \"dPadDown\"," \
"      \"inputType\": \"button\"," \
"      \"inputButtons\": \"down\"" \
"    }," \
"    {" \
"      \"simulatorType\": \"input\"," \
"      \"outputType\": \"xinput\"," \
"      \"xinputButtons\": \"dPadLeft\"," \
"      \"inputType\": \"button\"," \
"      \"inputButtons\": \"left\"" \
"    }," \
"    {" \
"      \"simulatorType\": \"input\"," \
"      \"outputType\": \"xinput\"," \
"      \"xinputButtons\": \"dPadRight\"," \
"      \"inputType\": \"button\"," \
"      \"inputButtons\": \"right\"" \
"    }," \
"    {" \
"      \"simulatorType\": \"input\"," \
"      \"outputType\": \"xinput\"," \
"      \"xinputButtons\": \"guide\"," \
"      \"inputType\": \"button\"," \
"      \"inputButtons\": \"ps\"" \
"    }," \
"    {" \
"      \"simulatorType\": \"input\"," \
"      \"outputType\": \"xinput\"," \
"      \"xinputAxes\": {" \
"        \"axes\": \"leftStickY\"," \
"        \"options\": {" \
"          \"leftStickY\": {" \
"            \"polarity\": \"positive\"" \
"          }" \
"        }" \
"      }," \
"      \"inputType\": \"axis\"," \
"      \"inputAxis\": \"leftStickY\"," \
"      \"inputAxisOptions\": {" \
"        \"leftStickY\": {" \
"          \"polarity\": \"positive\"" \
"        }" \
"      }" \
"    }," \
"    {" \
"      \"simulatorType\": \"input\"," \
"      \"outputType\": \"xinput\"," \
"      \"xinputAxes\": {" \
"        \"axes\": \"leftStickY\"," \
"        \"options\": {" \
"          \"leftStickY\": {" \
"            \"polarity\": \"negative\"" \
"          }" \
"        }" \
"      }," \
"      \"inputType\": \"axis\"," \
"      \"inputAxis\": \"leftStickY\"," \
"      \"inputAxisOptions\": {" \
"        \"leftStickY\": {" \
"          \"polarity\": \"negative\"" \
"        }" \
"      }" \
"    }," \
"    {" \
"      \"simulatorType\": \"input\"," \
"      \"outputType\": \"xinput\"," \
"      \"xinputAxes\": {" \
"        \"axes\": \"leftStickX\"," \
"        \"options\": {" \
"          \"leftStickX\": {" \
"            \"polarity\": \"positive\"" \
"          }" \
"        }" \
"      }," \
"      \"inputType\": \"axis\"," \
"      \"inputAxis\": \"leftStickX\"," \
"      \"inputAxisOptions\": {" \
"        \"leftStickX\": {" \
"          \"polarity\": \"positive\"" \
"        }" \
"      }" \
"    }," \
"    {" \
"      \"simulatorType\": \"input\"," \
"      \"outputType\": \"xinput\"," \
"      \"xinputAxes\": {" \
"        \"axes\": \"leftStickX\"," \
"        \"options\": {" \
"          \"leftStickX\": {" \
"            \"polarity\": \"negative\"" \
"          }" \
"        }" \
"      }," \
"      \"inputType\": \"axis\"," \
"      \"inputAxis\": \"leftStickX\"," \
"      \"inputAxisOptions\": {" \
"        \"leftStickX\": {" \
"          \"polarity\": \"negative\"" \
"        }" \
"      }" \
"    }," \
"    {" \
"      \"simulatorType\": \"input\"," \
"      \"outputType\": \"xinput\"," \
"      \"xinputAxes\": {" \
"        \"axes\": \"rightStickY\"," \
"        \"options\": {" \
"          \"rightStickY\": {" \
"            \"polarity\": \"positive\"" \
"          }" \
"        }" \
"      }," \
"      \"inputType\": \"axis\"," \
"      \"inputAxis\": \"rightStickY\"," \
"      \"inputAxisOptions\": {" \
"        \"rightStickY\": {" \
"          \"polarity\": \"positive\"" \
"        }" \
"      }" \
"    }," \
"    {" \
"      \"simulatorType\": \"input\"," \
"      \"outputType\": \"xinput\"," \
"      \"xinputAxes\": {" \
"        \"axes\": \"rightStickY\"," \
"        \"options\": {" \
"          \"rightStickY\": {" \
"            \"polarity\": \"negative\"" \
"          }" \
"        }" \
"      }," \
"      \"inputType\": \"axis\"," \
"      \"inputAxis\": \"rightStickY\"," \
"      \"inputAxisOptions\": {" \
"        \"rightStickY\": {" \
"          \"polarity\": \"negative\"" \
"        }" \
"      }" \
"    }," \
"    {" \
"      \"simulatorType\": \"input\"," \
"      \"outputType\": \"xinput\"," \
"      \"xinputAxes\": {" \
"        \"axes\": \"rightStickX\"," \
"        \"options\": {" \
"          \"rightStickX\": {" \
"            \"polarity\": \"positive\"" \
"          }" \
"        }" \
"      }," \
"      \"inputType\": \"axis\"," \
"      \"inputAxis\": \"rightStickX\"," \
"      \"inputAxisOptions\": {" \
"        \"rightStickX\": {" \
"          \"polarity\": \"positive\"" \
"        }" \
"      }" \
"    }," \
"    {" \
"      \"simulatorType\": \"input\"," \
"      \"outputType\": \"xinput\"," \
"      \"xinputAxes\": {" \
"        \"axes\": \"rightStickX\"," \
"        \"options\": {" \
"          \"rightStickX\": {" \
"            \"polarity\": \"negative\"" \
"          }" \
"        }" \
"      }," \
"      \"inputType\": \"axis\"," \
"      \"inputAxis\": \"rightStickX\"," \
"      \"inputAxisOptions\": {" \
"        \"rightStickX\": {" \
"          \"polarity\": \"negative\"" \
"        }" \
"      }" \
"    }," \
"    {" \
"      \"simulatorType\": \"input\"," \
"      \"outputType\": \"xinput\"," \
"      \"xinputAxes\": {" \
"        \"axes\": \"leftTrigger\"" \
"      }," \
"      \"inputType\": \"axis\"," \
"      \"inputAxis\": \"leftTrigger\"" \
"    }," \
"    {" \
"      \"simulatorType\": \"input\"," \
"      \"outputType\": \"xinput\"," \
"      \"xinputAxes\": {" \
"        \"axes\": \"rightTrigger\"" \
"      }," \
"      \"inputType\": \"axis\"," \
"      \"inputAxis\": \"rightTrigger\"" \
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

DeviceProfile DeviceProfile::defaultProfile()
{
	auto obj = QJsonDocument::fromJson(QString(defaultProfileJson).toUtf8());
	auto result = fromJson<DeviceProfile>(obj.object());
	return result;
}
