#include "pch.h"
#include "Trackball.h"

#pragma region unimportant

bool TrackballVibration::operator==(TrackballVibration& other) const
{
	return enabled == other.enabled &&
	       gmath::near_equal(factor, other.factor) &&
	       gmath::near_equal(deadZone, other.deadZone);
}

bool TrackballVibration::operator!=(TrackballVibration& other) const
{
	return !(*this == other);
}

void TrackballVibration::readJson(const nlohmann::json& json)
{
	enabled  = json["enabled"];
	factor   = json["factor"];
	deadZone = json["deadZone"];
}

void TrackballVibration::writeJson(nlohmann::json& json) const
{
	json["enabled"]  = enabled;
	json["factor"]   = factor;
	json["deadZone"] = deadZone;
}

bool TrackballSettings::operator==(TrackballSettings& other) const
{
	return touchVibration == other.touchVibration &&
	       ballVibration == other.ballVibration &&
	       gmath::near_equal(touchFriction, other.touchFriction) &&
	       gmath::near_equal(ballFriction, other.ballFriction) &&
	       gmath::near_equal(ballSpeed, other.ballSpeed);
}

bool TrackballSettings::operator!=(TrackballSettings& other) const
{
	return !(*this == other);
}

void TrackballSettings::readJson(const nlohmann::json& json)
{
	if (json.find("touchVibration") != json.end())
	{
		touchVibration = fromJson<TrackballVibration>(json["touchVibration"]);
	}

	if (json.find("ballVibration") != json.end())
	{
		ballVibration = fromJson<TrackballVibration>(json["ballVibration"]);
	}

	touchFriction = json["touchFriction"];
	ballFriction  = json["ballFriction"];
	ballSpeed     = json["ballSpeed"];
}

void TrackballSettings::writeJson(nlohmann::json& json) const
{
	json["touchVibration"] = touchVibration.toJson();
	json["ballVibration"]  = ballVibration.toJson();
	json["touchFriction"]  = touchFriction;
	json["ballFriction"]   = ballFriction;
	json["ballSpeed"]      = ballSpeed;
}

TrackballSimulator::TrackballSimulator(const TrackballSettings& settings, Ds4TouchRegion* region, InputSimulator* parent)
	: ISimulator(parent),
	  region(region),
	  rumbleTimer(std::make_shared<RumbleTimer>(parent, std::chrono::milliseconds(125), 0, 0)),
	  settings(settings)
{
}

#pragma endregion

bool TrackballSimulator::rolling() const
{
	return !gmath::is_zero(currentSpeed());
}

TrackballSimulator::TrackballState TrackballSimulator::applyDirectionalForce(Vector2 targetDirection, float factor, float deltaTime)
{
	if (gmath::is_zero(factor))
	{
		bool wasRolling = rolling();

		slow(deltaTime);

		if (wasRolling)
		{
			return TrackballState::slowing;
		}

		return TrackballState::stopped;
	}

	direction_ = targetDirection;
	accelerate(deltaTime, factor);
	return TrackballState::accelerating;
}

void TrackballSimulator::update(float deltaTime)
{
	doWork(deltaTime, Ds4Buttons::touch1);
	//doWork(deltaTime, Ds4Buttons::touch2);
}

void TrackballSimulator::accelerate(float deltaTime, float factor)
{
	currentSpeed_ = std::min(currentSpeed_ + (settings.ballSpeed * settings.touchFriction * factor * deltaTime), settings.ballSpeed);
}

void TrackballSimulator::slow(float deltaTime)
{
	currentSpeed_ = std::max(0.0f, currentSpeed_ - (settings.ballSpeed * settings.touchFriction * deltaTime));
}

void TrackballSimulator::decelerate(float deltaTime)
{
	currentSpeed_ = std::max(0.0f, currentSpeed_ - (settings.ballSpeed * settings.ballFriction * deltaTime));
}

template <size_t n>
void doStupidShit(const circular_buffer<Ds4TouchHistory, n>& points,
                std::optional<Ds4TouchHistory>& newest, std::optional<Ds4TouchHistory>& oldest)
{
	newest = points.newest();
	oldest = newest;

	// UNDONE: use this to calculate a flick threshold
	
	static constexpr auto threshold = std::chrono::milliseconds(125);

	for (size_t i = 0; i < points.count; i++)
	{
		if (newest->timestamp - points[i].timestamp < threshold)
		{
			oldest = points[i];
			break;
		}
	}
}

void TrackballSimulator::doWork(float deltaTime, Ds4Buttons_t touchId)
{
	auto width  = region->right - region->left;
	auto height = region->bottom - region->top;

	std::optional<Ds4TouchHistory> newest;
	std::optional<Ds4TouchHistory> oldest;
	
	if (region->isActive(touchId))
	{
		doStupidShit(region->getPoints(touchId), newest, oldest);
	}

	Vector2 direction = Vector2::zero;
	float force = 0.0f;

	if (newest.has_value() && oldest.has_value())
	{
		auto pa = newest->point;
		auto pb = oldest->point;

		region->clamp(pa);
		region->clamp(pb);

		direction = {
			static_cast<float>(pa.x - pb.x) / width,
			static_cast<float>(pa.y - pb.y) / height
		};

		force = direction.length();
		direction.normalize();
	}

	TrackballState state;

	if (region->isActive(touchId))
	{
		state = applyDirectionalForce(direction, force, deltaTime);
	}
	else
	{
		decelerate(deltaTime);
		state = rolling() ? TrackballState::decelerating : TrackballState::stopped;
	}

	const float f = currentSpeed_ / settings.ballSpeed;

	uint8_t left  = static_cast<uint8_t>(std::clamp(f * settings.touchVibration.factor * 255.0f, 0.0f, 255.0f));
	uint8_t right = static_cast<uint8_t>(std::clamp(f * settings.ballVibration.factor * 255.0f, 0.0f, 255.0f));

	switch (state)
	{
		case TrackballState::slowing:
			qDebug() << "slow";
			rumbleTimer->left = left;
			rumbleTimer->reset();
			parent->addSimulator(rumbleTimer.get());
			break;

		case TrackballState::decelerating:
			parent->setRumble(0, right);
			break;

		case TrackballState::accelerating:
			parent->setRumble(0, right);
			break;

		default:
			break;
	}
}
