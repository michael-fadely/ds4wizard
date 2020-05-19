#include "pch.h"
#include "Ds4TouchRegion.h"

ISimulator* Ds4TouchRegion::getSimulator(InputSimulator* parent)
{
	ISimulator* result = nullptr;

	switch (static_cast<Ds4TouchRegionType::_enumerated>(type))
	{
		case Ds4TouchRegionType::button:
		case Ds4TouchRegionType::stick:
		case Ds4TouchRegionType::stickAutoCenter:
			break;

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

std::optional<PressedState> Ds4TouchRegion::getSimulatorState() const
{
	std::optional<PressedState> result;

	switch (static_cast<Ds4TouchRegionType::_enumerated>(type))
	{
		case Ds4TouchRegionType::button:
		case Ds4TouchRegionType::stick:
		case Ds4TouchRegionType::stickAutoCenter:
			break;

		case Ds4TouchRegionType::trackball:
			result = trackball->rolling() ? PressedState::on : PressedState::off;
			break;

		default:
			break;
	}

	return result;
}

void Ds4TouchRegion::clamp(Ds4Vector2& point) const
{
	point.x = std::clamp(point.x, left, right);
	point.y = std::clamp(point.y, top, bottom);
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
	if (this == &other)
	{
		return *this;
	}
	
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

	if (allowCrossOver || !isTouchActive(sender))
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

bool Ds4TouchRegion::isTouchActive(Ds4Buttons_t sender) const
{
	return (activeButtons & (sender & (Ds4Buttons::touch1 | Ds4Buttons::touch2))) != 0;
}

bool Ds4TouchRegion::isActive(Ds4Buttons_t sender, Direction_t direction) const
{
	switch (type)
	{
		case Ds4TouchRegionType::none:
			return false;

		case Ds4TouchRegionType::button:
			return isTouchActive(sender);

		case Ds4TouchRegionType::stick:
		case Ds4TouchRegionType::stickAutoCenter:
		case Ds4TouchRegionType::trackball:
		{
			if (direction == Direction::none)
			{
				return isTouchActive(sender);
			}

			const auto simulatorState = getSimulatorState();

			if (simulatorState.has_value() && !Pressable::isActiveState(*simulatorState))
			{
				return false;
			}

			float axis = getSimulatedAxis(sender, direction);
			axis = applyDeadZone(direction, axis);
			return !gmath::is_zero(axis);
		}

		default:
			throw;
	}
}

void Ds4TouchRegion::activateTouch(Ds4Buttons_t sender, const Ds4Vector2& point)
{
	if ((sender & Ds4Buttons::touch1) != 0)
	{
		state1.press();
	}

	if ((sender & Ds4Buttons::touch2) != 0)
	{
		state2.press();
	}

	if (isTouchActive(sender))
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

void Ds4TouchRegion::deactivateTouch(Ds4Buttons_t sender, Ds4Vector2 point)
{
	activeButtons &= ~(sender & (Ds4Buttons::touch1 | Ds4Buttons::touch2));

	if ((sender & Ds4Buttons::touch1) != 0)
	{
		points1.insert(Ds4TouchHistory(point));
		state1.release();
	}

	if ((sender & Ds4Buttons::touch2) != 0)
	{
		points2.insert(Ds4TouchHistory(point));
		state2.release();
	}
}

float Ds4TouchRegion::getSimulatedAxis(Ds4Buttons_t sender, Direction_t direction) const
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
			const Vector2 normalized = trackball->velocity.normalized();
			const float length = trackball->velocity.length();
			const auto factor = trackball->settings.ballSpeed;

			switch (direction)
			{
				case Direction::up:
					result = std::clamp((normalized.y * length) / factor, 0.0f, 1.0f);
					break;

				case Direction::down:
					result = std::abs(std::clamp((normalized.y * length) / factor, -1.0f, 0.0f));
					break;

				case Direction::left:
					result = std::clamp((normalized.x * length) / factor, 0.0f, 1.0f);
					break;

				case Direction::right:
					result = std::abs(std::clamp((normalized.x * length) / factor, -1.0f, 0.0f));
					break;

				case Direction::none:
					result = (normalized * length).length() / factor;
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
					result = std::abs(std::clamp(std::clamp(y - cy, -height, height) / static_cast<float>(cy), -1.0f, 0.0f));
					break;

				case Direction::down:
					result = std::clamp(std::clamp(y - cy, -height, height) / static_cast<float>(cy), 0.0f, 1.0f);
					break;

				case Direction::left:
					result = std::abs(std::clamp(std::clamp(x - cx, -width, width) / static_cast<float>(cx), -1.0f, 0.0f));
					break;

				case Direction::right:
					result = std::clamp(std::clamp(x - cx, -width, width) / static_cast<float>(cx), 0.0f, 1.0f);
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
	const auto it = touchAxisOptions.find(direction);

	if (it == touchAxisOptions.end())
	{
		return 0.0f;
	}

	return touchAxisOptions[direction].deadZone.value_or(0.0f);
}

// UNDONE: update to be more like InputAxisOptions::applyToValue
float Ds4TouchRegion::applyDeadZone(Direction_t direction, float value) const
{
	const auto it = touchAxisOptions.find(direction);

	if (it == touchAxisOptions.end())
	{
		return value;
	}
	
	const InputAxisOptions& options = it->second;
	return options.applyToValue(value);
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
