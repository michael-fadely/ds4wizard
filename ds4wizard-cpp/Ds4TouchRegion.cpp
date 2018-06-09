#include "stdafx.h"
#include "Ds4TouchRegion.h"

Ds4TouchRegion::Ds4TouchRegion(Ds4TouchRegionType type, short left, short top, short right, short bottom, bool allowCrossOver)
{
	Type           = type;
	AllowCrossOver = allowCrossOver;
	Left           = left;
	Top            = top;
	Right          = right;
	Bottom         = bottom;
}

Ds4TouchRegion::Ds4TouchRegion(const Ds4TouchRegion& other)
{
	Type             = other.Type;
	AllowCrossOver   = other.AllowCrossOver;
	Left             = other.Left;
	Top              = other.Top;
	Right            = other.Right;
	Bottom           = other.Bottom;
	TouchAxisOptions = other.TouchAxisOptions;
}

bool Ds4TouchRegion::IsInRegion(Ds4Buttons_t sender, const Ds4Vector2& point) const
{
	return IsInRegion(sender, point.x, point.y);
}

bool Ds4TouchRegion::IsInRegion(Ds4Buttons_t sender, short x, short y) const
{
	if (x >= Left && x <= Right && y >= Top && y <= Bottom)
	{
		return true;
	}

	return !AllowCrossOver && IsActive(sender);
}

Ds4Vector2 Ds4TouchRegion::GetStartPoint(Ds4Buttons_t sender) const
{
	if ((sender & Ds4Buttons::touch1) != 0)
	{
		return PointStart1;
	}

	if ((sender & Ds4Buttons::touch2) != 0)
	{
		return PointStart2;
	}

	return {};
}

bool Ds4TouchRegion::IsActive(Ds4Buttons_t sender) const
{
	return (Active & sender) != 0;
}

void Ds4TouchRegion::SetActive(Ds4Buttons_t sender, const Ds4Vector2& point)
{
	if ((sender & Ds4Buttons::touch1) != 0)
	{
		State1.Press();
	}

	if ((sender & Ds4Buttons::touch2) != 0)
	{
		State2.Press();
	}

	if (IsActive(sender))
	{
		return;
	}

	Active |= sender;

	if ((sender & Ds4Buttons::touch1) != 0)
	{
		PointStart1 = point;
	}
	else if ((sender & Ds4Buttons::touch2) != 0)
	{
		PointStart2 = point;
	}
}

void Ds4TouchRegion::SetInactive(Ds4Buttons_t sender)
{
	Active &= ~sender;

	if ((sender & Ds4Buttons::touch1) != 0)
	{
		State1.Release();
	}

	if ((sender & Ds4Buttons::touch2) != 0)
	{
		State2.Release();
	}
}

float Ds4TouchRegion::GetTouchDelta(Ds4Buttons_t sender, Direction_t direction, const Ds4Vector2& point) const
{
	short x = std::clamp(point.x, Left, Right);
	short y = std::clamp(point.y, Top, Bottom);

	if (Type == +Ds4TouchRegionType::stickAutoCenter)
	{
		Ds4Vector2 start = GetStartPoint(sender);

		int width  = std::max(start.x - Left, Right - start.x);
		int height = std::max(start.y - Top, Bottom - start.y);

		short sx = start.x;
		short sy = start.y;

		float result;

		switch (direction)
		{
			case Direction::up:
				result = std::abs(std::clamp(std::clamp(y - sy, -height, height) / static_cast<float>(height), -1.0f, 0.0f));
				break;
			case Direction::down:
				result = std::clamp(std::clamp(y - sy, -height, height) / static_cast<float>(height), 0.0f, 1.0f);
				break;
			case Direction::left:
				result = std::abs(std::clamp(std::clamp(x - sx, -width, width) / static_cast<float>(width), -1.0f, 0.0f));
				break;
			case Direction::right:
				result = std::clamp(std::clamp(x - sx, -width, width) / static_cast<float>(width), 0.0f, 1.0f);
				break;
			default:
				throw /*new ArgumentOutOfRangeException(nameof(direction), direction, null) // TODO */;
		}

		return result;
	}
	else
	{
		x -= Left;
		y -= Top;

		int width  = Right - Left;
		int height = Bottom - Top;

		int cx = width / 2;
		int cy = height / 2;

		float result;

		switch (direction)
		{
			case Direction::up:
				//result = std::abs(((y - cy).Clamp(-height, height) / (float)cy).Clamp(-1.0f, 0.0f));
				result = std::abs(std::clamp(std::clamp(y - cy, -height, height) / static_cast<float>(cy), -1.0f, 0.f));
				break;
			case Direction::down:
				//result = ((y - cy).Clamp(-height, height) / (float)cy).Clamp(0.0f, 1.0f);
				result = std::clamp(std::clamp(y - cy, -height, height) / static_cast<float>(cy), 0.f, 1.f);
				break;
			case Direction::left:
				//result = std::abs(((x - cx).Clamp(-width, width) / (float)cx).Clamp(-1.0f, 0.0f));
				result = std::abs(std::clamp(std::clamp(x - cx, -width, width) / static_cast<float>(cx), -1.0f, 0.f));
				break;
			case Direction::right:
				//result = ((x - cx).Clamp(-width, width) / (float)cx).Clamp(0.0f, 1.0f);
				result = std::clamp(std::clamp(x - cx, -width, width) / static_cast<float>(cx), 0.f, 1.f);
				break;
			default:
				throw /*new ArgumentOutOfRangeException(nameof(direction), direction, null)*/;
		}

		return result;
	}
}

float Ds4TouchRegion::GetDeadZone(Direction_t direction)
{
	return TouchAxisOptions[direction].deadZone;
}

void Ds4TouchRegion::ApplyDeadZone(Direction_t direction, float& analog)
{
	InputAxisOptions options = TouchAxisOptions[direction];
	options.ApplyDeadZone(analog);
}

bool Ds4TouchRegion::operator==(const Ds4TouchRegion& other) const
{
	return AllowCrossOver == other.AllowCrossOver
	       && Left == other.Left
	       && Top == other.Top
	       && Right == other.Right
	       && Bottom == other.Bottom
	       && TouchAxisOptions == other.TouchAxisOptions;
}

bool Ds4TouchRegion::operator!=(const Ds4TouchRegion& other) const
{
	return !(*this == other);
}

void Ds4TouchRegion::readJson(const QJsonObject& json)
{
	Type           = Ds4TouchRegionType::_from_string(json["type"].toString().toStdString().c_str());
	AllowCrossOver = json["allowCrossOver"].toBool();
	Left           = json["left"].toInt();
	Top            = json["top"].toInt();
	Right          = json["right"].toInt();
	Bottom         = json["bottom"].toInt();

	auto touchAxisOptions_ = json["touchAxisOptions"].toObject();

	for (const auto& key : touchAxisOptions_.keys())
	{
		auto stdstr = key.toStdString();

		Direction_t value;
		ENUM_DESERIALIZE_FLAGS(Direction)(stdstr, value);

		TouchAxisOptions[value] = fromJson<InputAxisOptions>(touchAxisOptions_[key].toObject());
	}
}

void Ds4TouchRegion::writeJson(QJsonObject& json) const
{
	json["type"]           = Type._to_string();
	json["allowCrossOver"] = AllowCrossOver;
	json["left"]           = Left;
	json["top"]            = Top;
	json["right"]          = Right;
	json["bottom"]         = Bottom;

	QJsonObject touchAxisOptions_;

	for (const auto& pair : TouchAxisOptions)
	{
		touchAxisOptions_[ENUM_SERIALIZE_FLAGS(Direction)(pair.first).c_str()] = pair.second.toJson();
	}

	json["touchAxisOptions"] = touchAxisOptions_;
}
