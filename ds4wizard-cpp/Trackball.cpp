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

TrackballEmulator::TrackballEmulator(const TrackballSettings& settings)
	: settings(settings)
{
}

#pragma endregion

bool TrackballEmulator::rolling() const
{
	return gmath::is_zero(currentSpeed());
}

TrackballEmulator::TrackballState TrackballEmulator::applyDirectionalForce(Vector2 targetDirection, float factor, float deltaTime)
{
	if (gmath::is_zero(factor))
	{
		slow(deltaTime);
		return rolling() ? TrackballState::slowing : TrackballState::stopped;
	}

#if 0
	// slow first to help bias towards the desired direction
	slow(deltaTime);

	const Vector2 currentVelocity = direction_ * currentSpeed_;
	const Vector2 targetVelocity = direction * (factor * settings.ballSpeed * settings.touchFriction);

	// calculate the combined normalized direction
	direction_ = (currentVelocity + targetVelocity).normalized();
#else
	direction_ = targetDirection;
#endif

	accelerate(factor, deltaTime);

	return TrackballState::accelerating;
}

TrackballEmulator::TrackballState TrackballEmulator::update(float deltaTime)
{
	decelerate(deltaTime);

	return rolling() ? TrackballState::decelerating : TrackballState::stopped;
}

void TrackballEmulator::accelerate(float factor, float deltaTime)
{
	currentSpeed_ = std::min(currentSpeed_ + (settings.ballSpeed * settings.touchFriction * factor * deltaTime), settings.ballSpeed);
}

void TrackballEmulator::slow(float deltaTime)
{
	currentSpeed_ = std::max(0.0f, currentSpeed_ - (settings.ballSpeed * settings.touchFriction * deltaTime));
}

void TrackballEmulator::decelerate(float deltaTime)
{
	currentSpeed_ = std::max(0.0f, currentSpeed_ - (settings.ballSpeed * settings.ballFriction * deltaTime));
}
