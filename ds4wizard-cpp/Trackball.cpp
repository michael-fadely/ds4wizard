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

Vector2 make_good(const Vector2& v)
{
	const auto length = v.length();
	const auto normalized = v.normalized();
	return length * normalized;
}

bool TrackballSimulator::rolling() const
{
	return !gmath::is_zero(velocity.length());
}

TrackballSimulator::TrackballState TrackballSimulator::applyDirectionalForce(bool touching, Vector2 targetDirection, float force, float deltaTime)
{
	if (gmath::is_zero(force))
	{
		if (rolling())
		{
			if (touching)
			{
				slow(deltaTime);
				return TrackballState::slowing;
			}

			decelerate(deltaTime);
			return TrackballState::decelerating;
		}

		return TrackballState::stopped;
	}

	const auto currentDirection = velocity.normalized();

	if (targetDirection.near_equal(Vector2::zero))
	{
		targetDirection = currentDirection;
	}
	else if (!targetDirection.is_normalized())
	{
		targetDirection.normalize();
	}

	const float ballSpeed = settings.ballSpeed;
	const float friction = touching ? settings.touchFriction : settings.ballFriction;

	auto dot = currentDirection.dot(targetDirection);

	constexpr float threshold = 0.707107f;

	TrackballState result;

	if (std::abs(dot) >= threshold)
	{
		const Vector2 target  = make_good(targetDirection * (ballSpeed * force * friction * deltaTime));
		const Vector2 clamped = make_good(Vector2::clamp(velocity + target, -ballSpeed, ballSpeed));

		velocity = clamped;
		result = TrackballState::accelerating;
	}
	else
	{
		if (touching)
		{
			slow(deltaTime);
			result = TrackballState::slowing;
		}
		else
		{
			decelerate(deltaTime);
			result = TrackballState::decelerating;
		}

		const Vector2 target  = make_good(targetDirection * (ballSpeed * force * friction * deltaTime));
		const Vector2 clamped = make_good(Vector2::clamp(velocity - target, -ballSpeed, ballSpeed));

		velocity = clamped;
	}

	return result;
}

void TrackballSimulator::update(float deltaTime)
{
	simulate(deltaTime, Ds4Buttons::touch1);
	//simulate(deltaTime, Ds4Buttons::touch2);
}

void TrackballSimulator::accelerate(const Vector2& direction, float factor, float deltaTime)
{
	const float m = settings.ballSpeed * settings.touchFriction * factor * deltaTime;

	Vector2 targetVelocity = make_good(direction * m);
	Vector2 currentVelocity = make_good(velocity);

	auto combined = Vector2::clamp(targetVelocity + currentVelocity, -settings.ballSpeed, settings.ballSpeed);

	combined = make_good(combined);
	velocity = combined;
}

void TrackballSimulator::slow(float deltaTime)
{
	const float m = settings.ballSpeed * settings.touchFriction * deltaTime;
	const Vector2 direction = velocity.normalized();
	const Vector2 changed = velocity - (direction * m);

	if (velocity.length_squared() - changed.length_squared() < 0.0f)
	{
		velocity = Vector2::zero;
	}
	else
	{
		velocity = changed;
	}

	velocity = Vector2::clamp(velocity, -settings.ballSpeed, settings.ballSpeed);
}

void TrackballSimulator::decelerate(float deltaTime)
{
	const float m = settings.ballSpeed * settings.ballFriction * deltaTime;
	const Vector2 direction = velocity.normalized();
	const Vector2 changed = velocity - (direction * m);

	if (velocity.length_squared() - changed.length_squared() < 0.0f)
	{
		velocity = Vector2::zero;
	}
	else
	{
		velocity = changed;
	}

	velocity = Vector2::clamp(velocity, -settings.ballSpeed, settings.ballSpeed);
}

template <size_t N>
void doStupidShit(const circular_buffer<Ds4TouchHistory, N>& points,
                  std::optional<Ds4TouchHistory>& newest, std::optional<Ds4TouchHistory>& oldest)
{
	newest = points.newest();
	oldest = points.oldest();

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

void TrackballSimulator::simulate(float deltaTime, Ds4Buttons_t touchId)
{
	const short width  = static_cast<short>(region->right - region->left);
	const short height = static_cast<short>(region->bottom - region->top);

	std::optional<Ds4TouchHistory> newest;
	std::optional<Ds4TouchHistory> oldest;

	const bool touching = region->isTouchActive(touchId);
	
	if (touching)
	{
		doStupidShit(region->getPoints(touchId), newest, oldest);
	}

	Vector2 direction {};
	float force = 0.0f;

	if (newest.has_value() && oldest.has_value())
	{
		auto pa = newest->point;
		auto pb = oldest->point;

		region->clamp(pa);
		region->clamp(pb);

		direction = {
			static_cast<float>(pb.x - pa.x) / width,
			static_cast<float>(pb.y - pa.y) / height
		};

		force = direction.length();
		direction.normalize();
	}

	TrackballState state = applyDirectionalForce(touching, direction, force, deltaTime);

	const float f = velocity.length() / settings.ballSpeed;

	uint8_t left  = static_cast<uint8_t>(std::clamp(f * settings.touchVibration.factor * 255.0f, 0.0f, 255.0f));
	uint8_t right = static_cast<uint8_t>(std::clamp(f * settings.ballVibration.factor * 255.0f, 0.0f, 255.0f));

	switch (state)
	{
		case TrackballState::slowing:
			//qDebug() << "slowing";
			rumbleTimer->left = left;
			rumbleTimer->reset();
			parent->addSimulator(rumbleTimer.get());
			break;

		case TrackballState::decelerating:
			//qDebug() << "decelerating";
			parent->setRumble(0, right);
			break;

		case TrackballState::accelerating:
			//qDebug() << "accelerating";
			parent->setRumble(0, right);
			break;

		default:
			break;
	}
}
