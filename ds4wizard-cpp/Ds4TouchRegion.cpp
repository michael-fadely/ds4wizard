#include "pch.h"
#include "Ds4TouchRegion.h"

ISimulator* Ds4TouchRegion::getSimulator(InputSimulator* parent)
{
	ISimulator* result = nullptr;

	switch (static_cast<Ds4TouchRegionType::_enumerated>(type))
	{
		/*case Ds4TouchRegionType::button:
			break;

		case Ds4TouchRegionType::stick:
			break;

		case Ds4TouchRegionType::stickAutoCenter:
			break;*/

		case Ds4TouchRegionType::trackball:
			if (!trackball)
			{
				trackball = std::make_shared<TrackballSimulator>(*trackballSettings, this, parent);
			}

			result = trackball.get();
			break;

		default:
			break;
	}

	return result;
}

Ds4TouchRegion::Ds4TouchRegion() = default;

Ds4TouchRegion::Ds4TouchRegion(Ds4TouchRegionType type, short left, short top, short right, short bottom, bool allowCrossOver)
	: trackballSettings(nullptr),
	  type(type),
	  allowCrossOver(allowCrossOver),
	  left(left),
	  top(top),
	  right(right),
	  bottom(bottom)
{
}

Ds4TouchRegion::Ds4TouchRegion(const Ds4TouchRegion& other)
	: trackballSettings(other.trackballSettings),
	  type(other.type),
	  allowCrossOver(other.allowCrossOver),
	  left(other.left),
	  top(other.top),
	  right(other.right),
	  bottom(other.bottom),
	  touchAxisOptions(other.touchAxisOptions)
{
}

Ds4TouchRegion& Ds4TouchRegion::operator=(const Ds4TouchRegion& other)
{
	trackballSettings = other.trackballSettings;
	type              = other.type;
	allowCrossOver    = other.allowCrossOver;
	left              = other.left;
	top               = other.top;
	right             = other.right;
	bottom            = other.bottom;
	touchAxisOptions  = other.touchAxisOptions;
	
	return *this;
}

bool Ds4TouchRegion::isInRegion(Ds4Buttons_t sender, const Ds4Vector2& point)
{
	if (sender & Ds4Buttons::touch1)
	{
		points1.insert(Ds4TouchHistory(point));
	}

	if (sender & Ds4Buttons::touch2)
	{
		points2.insert(Ds4TouchHistory(point));
	}
	
	if (point.x >= left && point.x <= right && point.y >= top && point.y <= bottom)
	{
		return true;
	}

	if (allowCrossOver || !isActive(sender))
	{
		return false;
	}

	return true;
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
		points1.fill(Ds4TouchHistory(point));
	}
	else if ((sender & Ds4Buttons::touch2) != 0)
	{
		pointStart2 = point;
		points2.fill(Ds4TouchHistory(point));
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

// HACK: why should you be able to provide a touch point??? This should be handled internally given the last updated touch point!
float Ds4TouchRegion::getTouchDelta(Ds4Buttons_t sender, Direction_t direction) const
{
	Ds4Vector2 point {};

	if (sender & Ds4Buttons::touch1)
	{
		point = points1.newest().point;
	}
	else if (sender & Ds4Buttons::touch2)
	{
		point = points2.newest().point;
	}

	short x = std::clamp(point.x, left, right);
	short y = std::clamp(point.y, top, bottom);
	float result;

	/*
	 * HACK: this is so disjointed!
	 * Why is the emulation type specified here *at all*?
	 * You should be able to ask for a delta kind (from
	 * region center, from start, from data points) and
	 * be done!
	 */
	switch (type)
	{
		case +Ds4TouchRegionType::stickAutoCenter:
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
					throw std::runtime_error("invalid Direction");
			}

			break;
		}

		case +Ds4TouchRegionType::trackball:
		{
			switch (direction)
			{
				case Direction::up:
					result = std::clamp((trackball->direction().y * trackball->currentSpeed()) / trackball->settings.ballSpeed, 0.0f, 1.0f);
					break;

				case Direction::down:
					result = std::abs(std::clamp((trackball->direction().y * trackball->currentSpeed()) / trackball->settings.ballSpeed, -1.0f, 0.0f));
					break;

				case Direction::left:
					result = std::clamp((trackball->direction().x * trackball->currentSpeed()) / trackball->settings.ballSpeed, 0.0f, 1.0f);
					break;

				case Direction::right:
					result = std::abs(std::clamp((trackball->direction().x * trackball->currentSpeed()) / trackball->settings.ballSpeed, -1.0f, 0.0f));
					break;

				case Direction::none:
					result = (trackball->direction() * trackball->currentSpeed()).length() / trackball->settings.ballSpeed;
					break;

				default:
					throw std::runtime_error("invalid Direction");
			}

			break;
		}

		default:
		{
			x = static_cast<short>(x - left);
			y = static_cast<short>(y - top);

			int width  = right - left;
			int height = bottom - top;

			int cx = width / 2;
			int cy = height / 2;

			switch (direction)
			{
				case Direction::up:
					result = std::abs(std::clamp(std::clamp(y - cy, -height, height) / static_cast<float>(cy), -1.0f, 0.f));
					break;

				case Direction::down:
					result = std::clamp(std::clamp(y - cy, -height, height) / static_cast<float>(cy), 0.f, 1.f);
					break;

				case Direction::left:
					result = std::abs(std::clamp(std::clamp(x - cx, -width, width) / static_cast<float>(cx), -1.0f, 0.f));
					break;

				case Direction::right:
					result = std::clamp(std::clamp(x - cx, -width, width) / static_cast<float>(cx), 0.f, 1.f);
					break;

				default:
					throw std::runtime_error("invalid Direction");
			}
			break;
		}
	}

	return result;
}

const decltype(Ds4TouchRegion::points1)& Ds4TouchRegion::getPoints(Ds4Buttons_t sender) const
{
	if (sender & Ds4Buttons::touch1)
	{
		return points1;
	}

	if (sender & Ds4Buttons::touch2)
	{
		return points2;
	}

	throw;
}

// HACK: why does this just provide the dead zone? Why doesn't it do its own simulation?
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

void Ds4TouchRegion::readJson(const nlohmann::json& json)
{
	type           = Ds4TouchRegionType::_from_string(json["type"].get<std::string>().c_str());
	allowCrossOver = json["allowCrossOver"];
	left           = json["left"];
	top            = json["top"];
	right          = json["right"];
	bottom         = json["bottom"];

	auto it = json.find("touchAxisOptions");

	if (it != json.end())
	{
		auto touchAxisOptions_ = it->items();

		for (const auto& pair : touchAxisOptions_)
		{
			Direction_t value;
			ENUM_DESERIALIZE_FLAGS(Direction)(pair.key(), value);
			touchAxisOptions[value] = fromJson<InputAxisOptions>(pair.value());
		}
	}

	it = json.find("trackballSettings");

	if (it != json.end())
	{
		trackballSettings = std::make_shared<TrackballSettings>(fromJson<TrackballSettings>(*it));
	}
}

void Ds4TouchRegion::writeJson(nlohmann::json& json) const
{
	json["type"]           = type._to_string();
	json["allowCrossOver"] = allowCrossOver;
	json["left"]           = left;
	json["top"]            = top;
	json["right"]          = right;
	json["bottom"]         = bottom;

	nlohmann::json touchAxisOptions_;

	for (const auto& pair : touchAxisOptions)
	{
		touchAxisOptions_[ENUM_SERIALIZE_FLAGS(Direction)(pair.first)] = pair.second.toJson();
	}

	json["touchAxisOptions"] = touchAxisOptions_;

	if (trackballSettings != nullptr)
	{
		json["trackballSettings"] = trackballSettings->toJson();
	}
}
