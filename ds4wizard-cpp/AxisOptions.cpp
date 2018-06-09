#include "stdafx.h"
#include "AxisOptions.h"

AxisOptions::AxisOptions(AxisPolarity polarity)
{
	polarity = polarity;
}

AxisOptions::AxisOptions(const AxisOptions& other)
{
	multiplier = other.multiplier;
	polarity   = other.polarity;
}

bool AxisOptions::operator==(const AxisOptions& other) const
{
	return multiplier == other.multiplier && polarity == other.polarity;
}

bool AxisOptions::operator!=(const AxisOptions& other) const
{
	return !(*this == other);
}

void AxisOptions::readJson(const QJsonObject& json)
{
	if (json.contains("multiplier"))
	{
		multiplier = static_cast<float>(json["multiplier"].toDouble(1.0));
	}

	if (json.contains("polarity"))
	{
		polarity = AxisPolarity::_from_string(json["polarity"].toString("none").toStdString().c_str());
	}
}

void AxisOptions::writeJson(QJsonObject& json) const
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

InputAxisOptions::InputAxisOptions(const InputAxisOptions& other)
	: AxisOptions(other)
{
	invert       = other.invert;
	deadZoneMode = other.deadZoneMode;
	deadZone     = other.deadZone;
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
			throw /*new ArgumentOutOfRangeException(nameof(DeadZoneMode), DeadZoneMode, "Invalid deadzone mode.")*/;
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

void InputAxisOptions::readJson(const QJsonObject& json)
{
	AxisOptions::readJson(json);

	if (json.contains("invert"))
	{
		invert = json["invert"].toBool(false);
	}

	if (json.contains("deadZoneMode"))
	{
		deadZoneMode = DeadZoneMode::_from_string(json["deadZoneMode"].toString("scale").toStdString().c_str());
	}

	if (json.contains("deadZone"))
	{
		deadZone = static_cast<float>(json["deadZone"].toDouble());
	}
}

void InputAxisOptions::writeJson(QJsonObject& json) const
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

XInputAxes::XInputAxes(const XInputAxes& other)
{
	axes = other.axes;

	if (!other.options.empty())
	{
		for (auto& pair : other.options)
		{
			options[pair.first] = pair.second;
		}
	}
}

AxisOptions XInputAxes::getAxisOptions(XInputAxis::T axis)
{
	if (options.empty())
	{
		return AxisOptions();
	}

	const auto it = options.find(axis);

	if (it != options.end())
	{
		return it->second;
	}

	// HACK: separate into its own cache
	AxisOptions result {};
	options[axis] = result;
	return result;
}

bool XInputAxes::operator==(const XInputAxes& other) const
{
	return axes == other.axes && options == other.options;
}

bool XInputAxes::operator!=(const XInputAxes& other) const
{
	return !(*this == other);
}

void XInputAxes::readJson(const QJsonObject& json)
{
	auto axes_ = json["axes"].toString("none").toStdString();

	ENUM_DESERIALIZE_FLAGS(XInputAxis)(axes_, axes);

	auto options_ = json["options"].toObject();

	for (auto& key : options_.keys())
	{
		auto stdstr = key.toStdString();
		XInputAxis_t value;
		ENUM_DESERIALIZE_FLAGS(XInputAxis)(stdstr, value);
		options[value] = fromJson<AxisOptions>(options_[key].toObject());
	}
}

void XInputAxes::writeJson(QJsonObject& json) const
{
	json["axes"] = ENUM_SERIALIZE_FLAGS(XInputAxis)(axes).c_str();

	QJsonObject options_;

	for (const auto& pair : options)
	{
		auto key = ENUM_SERIALIZE_FLAGS(XInputAxis)(pair.first);
		options_[key.c_str()] = pair.second.toJson();
	}

	json["options"] = options_;
}

MouseAxes::MouseAxes(const MouseAxes& other)
{
	directions = other.directions;

	if (!other.options.empty())
	{
		for (auto& pair : other.options)
		{
			options[pair.first] = pair.second;
		}
	}
}

AxisOptions MouseAxes::getAxisOptions(Direction_t axis)
{
	if (options.empty())
	{
		return AxisOptions();
	}

	const auto it = options.find(axis);

	if (it != options.end())
	{
		return it->second;
	}

	// HACK: separate into its own cache
	AxisOptions result {};
	options[axis] = result;
	return result;
}

bool MouseAxes::operator==(const MouseAxes& other) const
{
	return directions == other.directions
	       && options == other.options;
}

void MouseAxes::readJson(const QJsonObject& json)
{
	throw; // TODO
}

void MouseAxes::writeJson(QJsonObject& json) const
{
	throw; // TODO
}
