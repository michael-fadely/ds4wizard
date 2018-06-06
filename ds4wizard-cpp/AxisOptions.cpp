#include "stdafx.h"
#include "AxisOptions.h"

AxisOptions::AxisOptions(AxisPolarity polarity)
{
	Polarity = polarity;
}

AxisOptions::AxisOptions(const AxisOptions& other)
{
	Multiplier = other.Multiplier;
	Polarity   = other.Polarity;
}

bool AxisOptions::operator==(const AxisOptions& other) const
{
	return Multiplier == other.Multiplier && Polarity == other.Polarity;
}

bool AxisOptions::operator!=(const AxisOptions& other) const
{
	return !(*this == other);
}

void AxisOptions::readJson(const QJsonObject& json)
{
	Multiplier = static_cast<float>(json["multiplier"].toDouble(1.0));
	Polarity   = AxisPolarity::_from_string(json["polarity"].toString("none").toStdString().c_str()); // oh god

	if (Polarity == +AxisPolarity::negative)
	{
		qDebug() << "that's numberwang";
	}
}

void AxisOptions::writeJson(QJsonObject& json) const
{
	json["multiplier"] = Multiplier;
	json["polarity"] = Polarity._to_string();
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

void InputAxisOptions::ApplyDeadZone(float& analog) const
{
	switch (deadZoneMode)
	{
		case DeadZoneMode::hardLimit:
			analog = analog >= deadZone ? analog : 0.0f;
			break;

		case DeadZoneMode::scale:
			analog = std::max(0.0f, (analog - deadZone) / (1.0f - deadZone));
			break;

		default:
			throw /*new ArgumentOutOfRangeException(nameof(DeadZoneMode), DeadZoneMode, "Invalid deadzone mode.")*/;
	}

	if (invert)
	{
		analog = 1.0f - analog;
	}

	analog *= Multiplier;
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

	invert       = json["invert"].toBool(false);
	deadZoneMode = DeadZoneMode::_from_string(json["deadZoneMode"].toString("scale").toStdString().c_str()); // I feel dirty
	deadZone     = static_cast<float>(json["deadZone"].toDouble());
}

void InputAxisOptions::writeJson(QJsonObject& json) const
{
	AxisOptions::writeJson(json);

	json["invert"]       = invert;
	json["deadZoneMode"] = deadZoneMode._to_string();
	json["deadZone"]     = deadZone;
}

XInputAxes::XInputAxes(const XInputAxes& other)
{
	Axes = other.Axes;

	if (!other.Options.empty())
	{
		for (auto& pair : other.Options)
		{
			Options[pair.first] = pair.second;
		}
	}
}

AxisOptions XInputAxes::GetAxisOptions(XInputAxis::T axis)
{
	if (Options.empty())
	{
		return AxisOptions();
	}

	const auto it = Options.find(axis);

	if (it != Options.end())
	{
		return it->second;
	}

	// HACK: separate into its own cache
	AxisOptions result {};
	Options[axis] = result;
	return result;
}

bool XInputAxes::operator==(const XInputAxes& other) const
{
	return Axes == other.Axes && Options == other.Options;
}

bool XInputAxes::operator!=(const XInputAxes& other) const
{
	return !(*this == other);
}

void XInputAxes::readJson(const QJsonObject& json)
{
	auto axes_ = json["axes"].toString("none").toStdString();

	ENUM_DESERIALIZE_FLAGS(XInputAxis)(axes_, Axes);

	auto options_ = json["options"].toObject();

	for (auto& key : options_.keys())
	{
		auto stdstr = key.toStdString();
		XInputAxis_t value;
		ENUM_DESERIALIZE_FLAGS(XInputAxis)(stdstr, value);
		Options[value] = fromJson<AxisOptions>(options_[key].toObject());
	}
}

void XInputAxes::writeJson(QJsonObject& json) const
{
	json["axes"] = ENUM_SERIALIZE_FLAGS(XInputAxis)(Axes).c_str();

	QJsonObject options_;

	for (const auto& pair : Options)
	{
		auto key = ENUM_SERIALIZE_FLAGS(XInputAxis)(pair.first);
		options_[key.c_str()] = pair.second.toJson();
	}

	json["options"] = options_;
}

MouseAxes::MouseAxes(const MouseAxes& other)
{
	Directions = other.Directions;

	if (!other.Options.empty())
	{
		for (auto& pair : other.Options)
		{
			Options[pair.first] = pair.second;
		}
	}
}

AxisOptions MouseAxes::GetAxisOptions(Direction_t axis)
{
	if (Options.empty())
	{
		return AxisOptions();
	}

	const auto it = Options.find(axis);

	if (it != Options.end())
	{
		return it->second;
	}

	// HACK: separate into its own cache
	AxisOptions result {};
	Options[axis] = result;
	return result;
}

bool MouseAxes::operator==(const MouseAxes& other) const
{
	return Directions == other.Directions
	       && Options == other.Options;
}

void MouseAxes::readJson(const QJsonObject& json)
{
	throw; // TODO
}

void MouseAxes::writeJson(QJsonObject& json) const
{
	throw; // TODO
}
