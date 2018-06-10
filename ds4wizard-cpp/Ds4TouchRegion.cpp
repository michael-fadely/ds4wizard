#include "stdafx.h"
#include "Ds4TouchRegion.h"

Ds4TouchRegion::Ds4TouchRegion(Ds4TouchRegionType type, short left, short top, short right, short bottom, bool allowCrossOver)
	: activeButtons(0),
	  type(type),
	  allowCrossOver(allowCrossOver),
	  left(left),
	  top(top),
	  right(right),
	  bottom(bottom)
{
}

Ds4TouchRegion::Ds4TouchRegion(const Ds4TouchRegion& other)
	: activeButtons(0),
	  type(other.type),
	  allowCrossOver(other.allowCrossOver),
	  left(other.left),
	  top(other.top),
	  right(other.right),
	  bottom(other.bottom),
	  touchAxisOptions(other.touchAxisOptions)
{
}

bool Ds4TouchRegion::isInRegion(Ds4Buttons_t sender, const Ds4Vector2& point) const
{
	return isInRegion(sender, point.x, point.y);
}

bool Ds4TouchRegion::isInRegion(Ds4Buttons_t sender, short x, short y) const
{
	if (x >= left && x <= right && y >= top && y <= bottom)
	{
		return true;
	}

	return !allowCrossOver && isActive(sender);
}

Ds4Vector2 Ds4TouchRegion::getStartPoint(Ds4Buttons_t sender) const
{
	if ((sender & Ds4Buttons::touch1) != 0)
	{
		return pointStart1;
	}

	if ((sender & Ds4Buttons::touch2) != 0)
	{
		return pointStart2;
	}

	return {};
}

bool Ds4TouchRegion::isActive(Ds4Buttons_t sender) const
{
	return (activeButtons & (sender & (Ds4Buttons::touch1 | Ds4Buttons::touch2))) != 0;
}

void Ds4TouchRegion::setActive(Ds4Buttons_t sender, const Ds4Vector2& point)
{
	if ((sender & Ds4Buttons::touch1) != 0)
	{
		state1.press();
	}

	if ((sender & Ds4Buttons::touch2) != 0)
	{
		state2.press();
	}

	if (isActive(sender))
	{
		return;
	}

	activeButtons |= sender & (Ds4Buttons::touch1 | Ds4Buttons::touch2);

	if ((sender & Ds4Buttons::touch1) != 0)
	{
		pointStart1 = point;
	}
	else if ((sender & Ds4Buttons::touch2) != 0)
	{
		pointStart2 = point;
	}
}

void Ds4TouchRegion::setInactive(Ds4Buttons_t sender)
{
	activeButtons &= ~(sender & (Ds4Buttons::touch1 | Ds4Buttons::touch2));

	if ((sender & Ds4Buttons::touch1) != 0)
	{
		state1.release();
	}

	if ((sender & Ds4Buttons::touch2) != 0)
	{
		state2.release();
	}
}

float Ds4TouchRegion::getTouchDelta(Ds4Buttons_t sender, Direction_t direction, const Ds4Vector2& point) const
{
	short x = std::clamp(point.x, left, right);
	short y = std::clamp(point.y, top, bottom);
	float result;

	if (type == +Ds4TouchRegionType::stickAutoCenter)
	{
		Ds4Vector2 start = getStartPoint(sender);

		int width  = std::max(start.x - left, right - start.x);
		int height = std::max(start.y - top, bottom - start.y);

		short sx = start.x;
		short sy = start.y;

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
	}
	else
	{
		x -= left;
		y -= top;

		int width  = right - left;
		int height = bottom - top;

		int cx = width / 2;
		int cy = height / 2;

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
	}

	return result;
}

float Ds4TouchRegion::getDeadZone(Direction_t direction)
{
	return touchAxisOptions[direction].deadZone.value_or(0.0f);
}

void Ds4TouchRegion::applyDeadZone(Direction_t direction, float& analog)
{
	InputAxisOptions options = touchAxisOptions[direction];
	options.applyDeadZone(analog);
}

bool Ds4TouchRegion::operator==(const Ds4TouchRegion& other) const
{
	return allowCrossOver == other.allowCrossOver
	       && left == other.left
	       && top == other.top
	       && right == other.right
	       && bottom == other.bottom
	       && touchAxisOptions == other.touchAxisOptions;
}

bool Ds4TouchRegion::operator!=(const Ds4TouchRegion& other) const
{
	return !(*this == other);
}

void Ds4TouchRegion::readJson(const QJsonObject& json)
{
	type           = Ds4TouchRegionType::_from_string(json["type"].toString().toStdString().c_str());
	allowCrossOver = json["allowCrossOver"].toBool();
	left           = json["left"].toInt();
	top            = json["top"].toInt();
	right          = json["right"].toInt();
	bottom         = json["bottom"].toInt();

	auto touchAxisOptions_ = json["touchAxisOptions"].toObject();

	for (const auto& key : touchAxisOptions_.keys())
	{
		const std::string stdstr = key.toStdString();

		Direction_t value;
		ENUM_DESERIALIZE_FLAGS(Direction)(stdstr, value);

		touchAxisOptions[value] = fromJson<InputAxisOptions>(touchAxisOptions_[key].toObject());
	}
}

void Ds4TouchRegion::writeJson(QJsonObject& json) const
{
	json["type"]           = type._to_string();
	json["allowCrossOver"] = allowCrossOver;
	json["left"]           = left;
	json["top"]            = top;
	json["right"]          = right;
	json["bottom"]         = bottom;

	QJsonObject touchAxisOptions_;

	for (const auto& pair : touchAxisOptions)
	{
		touchAxisOptions_[ENUM_SERIALIZE_FLAGS(Direction)(pair.first).c_str()] = pair.second.toJson();
	}

	json["touchAxisOptions"] = touchAxisOptions_;
}
