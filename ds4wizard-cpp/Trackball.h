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
	Ds4TouchRegion* region;
	std::shared_ptr<RumbleTimer> rumbleTimer;

public:
	TrackballSettings settings;

	TrackballSimulator(const TrackballSettings& settings, Ds4TouchRegion* region, InputSimulator* parent);

	Vector2 velocity {};
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
	 *        If \p force is zero, the ball will slow down according to \p touching.
	 *        If \p touching is \c false, standard friction is applied.
	 *        Otherwise, touch friction is applied.
	 * \param touching Indicates that the ball is being touched.
	 * \param targetDirection The direction to apply the force.
	 * \param force The amount of \c settings.ballSpeed to apply.
	 * \param deltaTime Delta time to be used in acceleration and deceleration.
	 * \return A \c TrackballState indicating the state of the ball after the call.
	 */
	TrackballState applyDirectionalForce(bool touching, Vector2 targetDirection, float force, float deltaTime);

	/**
	 * \brief Update the ball without any external influence.
	 * \param deltaTime Delta time to be used in acceleration and deceleration.
	 * \return A \c TrackballState indicating the state of the ball after the call.
	 */
	void update(float deltaTime) override;

private:
	/** \brief Accelerate the ball! */
	void accelerate(const Vector2& direction, float factor, float deltaTime);
	/** \brief Assume ball is being touched and slow the ball to a stop. */
	void slow(float deltaTime);
	/** \brief Allow the ball to naturally decelerate with nothing but friction. */
	void decelerate(float deltaTime);
	
	void simulate(float deltaTime, Ds4Buttons_t touchId);
};
