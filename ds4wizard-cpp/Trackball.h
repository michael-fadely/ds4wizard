#pragma once
#include "Vector2.h"
#include "ISimulator.h"
#include "RumbleSequence.h"

class Ds4TouchRegion;

struct TrackballVibration : JsonData
{
	bool  enabled  = false;
	float factor   = 1.0f;
	float deadZone = 0.0f;

	bool operator==(TrackballVibration& other) const;
	bool operator!=(TrackballVibration& other) const;

	void readJson(const nlohmann::json& json) override;
	void writeJson(nlohmann::json& json) const override;
};

struct TrackballSettings : JsonData
{
	TrackballVibration touchVibration;
	TrackballVibration ballVibration;

	float touchFriction = 1.0f;
	float ballFriction  = 0.025f;

	float ballSpeed = 100.0f;

	bool operator==(TrackballSettings& other) const;
	bool operator!=(TrackballSettings& other) const;

	void readJson(const nlohmann::json& json) override;
	void writeJson(nlohmann::json& json) const override;
};

class TrackballSimulator : public ISimulator
{
	Vector2 direction_ {};
	float currentSpeed_ = 0.0f;
	Ds4TouchRegion* region;
	std::shared_ptr<RumbleTimer> rumbleTimer;

public:
	TrackballSettings settings;

	TrackballSimulator(const TrackballSettings& settings, Ds4TouchRegion* region, InputSimulator* parent);

	[[nodiscard]] Vector2 direction() const { return direction_; }
	[[nodiscard]] float currentSpeed() const { return currentSpeed_; }
	[[nodiscard]] bool rolling() const;

	/**
	 * \brief Indicates the state of the emulated trackball.
	 */
	enum class TrackballState
	{
		/** \brief Ball is not moving. */
		stopped,
		/** \brief Ball is being slowed by a touch. */
		slowing,
		/** \brief Ball is naturally decelerating without external influence. */
		decelerating,
		/** \brief Ball is accelerating! */
		accelerating
	};

	/**
	 * \brief Apply a force to the ball in a given direction.
	 *        If \p factor is zero, it is assumed that the ball is being
	 *        explicitly slowed by an external force (i.e. touch).
	 * \param targetDirection The direction to apply the force.
	 * \param factor The amount of \c settings.ballSpeed to apply.
	 * \param deltaTime Delta time to be used in acceleration and deceleration.
	 * \return A \c TrackballState indicating the state of the ball after the call.
	 */
	TrackballState applyDirectionalForce(Vector2 targetDirection, float factor, float deltaTime);

	/**
	 * \brief Update the ball without any external influence.
	 * \param deltaTime Delta time to be used in acceleration and deceleration.
	 * \return A \c TrackballState indicating the state of the ball after the call.
	 */
	void update(float deltaTime) override;

private:
	/** \brief Accelerate the ball! */
	void accelerate(float deltaTime, float factor);
	/** \brief Assume ball is being touched and slow the ball to a stop. */
	void slow(float deltaTime);
	/** \brief Allow the ball to naturally decelerate with nothing but friction. */
	void decelerate(float deltaTime);

	void doWork(float deltaTime, Ds4Buttons_t touchId);
};
