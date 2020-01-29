#include "pch.h"
#include "AxisOptions.h"

AxisOptions::AxisOptions(AxisPolarity polarity)
	: polarity(polarity)
{
}

bool AxisOptions::operator==(const AxisOptions& other) const
{
	return multiplier == other.multiplier && polarity == other.polarity;
}

bool AxisOptions::operator!=(const AxisOptions& other) const
{
	return !(*this == other);
}

void AxisOptions::readJson(const nlohmann::json& json)
{
	if (json.find("multiplier") != json.end())
	{
		multiplier = json["multiplier"].get<float>();
	}

	if (json.find("polarity") != json.end())
	{
		polarity = AxisPolarity::_from_string(json["polarity"].get<std::string>().c_str());
	}
}

void AxisOptions::writeJson(nlohmann::json& json) const
{
	if (multiplier.has_value())
	{
		json["multiplier"] = multiplier.value();
	}

	if (polarity.has_value())
	{
		json["polarity"] = polarity.value()._to_string();
	}
}

InputAxisOptions::InputAxisOptions(AxisPolarity polarity)
	: AxisOptions(polarity)
{
}

void InputAxisOptions::applyDeadZone(float& analog) const
{
	const auto dzValue = deadZone.value_or(0.0f);

	switch (deadZoneMode.value_or(DeadZoneMode::scale))
	{
		case DeadZoneMode::hardLimit:
			analog = analog >= dzValue ? analog : 0.0f;
			break;

		case DeadZoneMode::scale:
			analog = std::max(0.0f, (analog - dzValue) / (1.0f - dzValue));
			break;

		default:
			throw std::out_of_range("invalid DeadZoneMode");
	}

	if (invert.value_or(false))
	{
		analog = 1.0f - analog;
	}

	if (multiplier.has_value())
	{
		analog *= multiplier.value();
	}
}

bool InputAxisOptions::operator==(const InputAxisOptions& other) const
{
	return AxisOptions::operator==(other)
	       && invert == other.invert
	       && deadZoneMode == other.deadZoneMode
	       && deadZone == other.deadZone;
}

bool InputAxisOptions::operator!=(const InputAxisOptions& other) const
{
	return !(*this == other);
}

void InputAxisOptions::readJson(const nlohmann::json& json)
{
	AxisOptions::readJson(json);

	if (json.find("invert") != json.end())
	{
		invert = json["invert"].get<bool>();
	}

	if (json.find("deadZoneMode") != json.end())
	{
		deadZoneMode = DeadZoneMode::_from_string(json.value("deadZoneMode", "scale").c_str());
	}

	if (json.find("deadZone") != json.end())
	{
		deadZone = json["deadZone"].get<float>();
	}
}

void InputAxisOptions::writeJson(nlohmann::json& json) const
{
	AxisOptions::writeJson(json);

	if (invert.has_value())
	{
		json["invert"] = invert.value();
	}

	if (deadZoneMode.has_value())
	{
		json["deadZoneMode"] = deadZoneMode.value()._to_string();
	}

	if (deadZone.has_value())
	{
		json["deadZone"] = deadZone.value();
	}
}

AxisOptions XInputAxes::getAxisOptions(XInputAxis::T axis) const
{
	const auto it = options.find(axis);

	if (it == options.end())
	{
		return AxisOptions();
	}

	return it->second;
}

bool XInputAxes::operator==(const XInputAxes& other) const
{
	return axes == other.axes && options == other.options;
}

bool XInputAxes::operator!=(const XInputAxes& other) const
{
	return !(*this == other);
}

void XInputAxes::readJson(const nlohmann::json& json)
{
	auto axes_ = json.value("axes", "none");

	ENUM_DESERIALIZE_FLAGS(XInputAxis)(axes_, axes);

	if (json.find("options") != json.end())
	{
		auto options_ = json["options"].items();

		for (auto& pair : options_)
		{
			XInputAxis_t value;
			ENUM_DESERIALIZE_FLAGS(XInputAxis)(pair.key(), value);
			options[value] = fromJson<AxisOptions>(pair.value());
		}
	}
}

void XInputAxes::writeJson(nlohmann::json& json) const
{
	if (axes != XInputAxis::none)
	{
		json["axes"] = ENUM_SERIALIZE_FLAGS(XInputAxis)(axes).c_str();
	}

	if (!options.empty())
	{
		nlohmann::json options_;

		for (const auto& pair : options)
		{
			auto key = ENUM_SERIALIZE_FLAGS(XInputAxis)(pair.first);
			options_[key.c_str()] = pair.second.toJson();
		}

		json["options"] = options_;
	}
}

AxisOptions MouseAxes::getAxisOptions(Direction_t axis)
{
	const auto it = options.find(axis);

	if (it == options.end())
	{
		return AxisOptions();
	}

	return it->second;
}

bool MouseAxes::operator==(const MouseAxes& other) const
{
	return directions == other.directions
	       && options == other.options;
}

void MouseAxes::readJson(const nlohmann::json& /*json*/)
{
	// TODO: not implemented
	throw std::runtime_error(__FUNCTION__ " not implemented");
}

void MouseAxes::writeJson(nlohmann::json& /*json*/) const
{
	// TODO: not implemented
	throw std::runtime_error(__FUNCTION__ " not implemented");
}
