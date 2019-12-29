#pragma once

#include <unordered_map>
#include <chrono>

#include <enum.h>

#include "enums.h"
#include "Ds4InputData.h"
#include "Pressable.h"
#include "AxisOptions.h"
#include "JsonData.h"
#include "circular_buffer.h"
#include "Trackball.h"

// TODO: TrackPad - basically always auto-centering stick
// TODO: Slider (maybe just "cursor", with toggle-able X and Y axes?)
// TODO: Make auto-center an option instead of type?

/*
 * How do we detect a touch pad flick?
 * - points[newest] - points[oldest], where points = circular buffer of last N touch points
 */

BETTER_ENUM(Ds4TouchRegionType, int,
            none,
            button,
            stick,
            stickAutoCenter,
            trackball)

class Ds4TouchRegion;

// TODO: make these use sorted maps

/**
 * \brief A collection of \c Ds4TouchRegion
 * \sa Ds4TouchRegion
 */
using Ds4TouchRegionCollection = std::unordered_map<std::string, Ds4TouchRegion>;

/**
 * \brief A collection of \c Ds4TouchRegion* as a caching mechanism.
 * \sa Ds4TouchRegion, Ds4TouchRegionCollection
 */
using Ds4TouchRegionCache = std::unordered_map<std::string, Ds4TouchRegion*>;

/*
 * for when you inevitably come to this code and think:
 * "why doesn't this have InputModifier support?"
 *
 * - a Ds4TouchRegion is a virtual input, not a mapping;
 * - InputMapBase can use Ds4TouchRegion as an input;
 * - an InputMap which is part of an InputModifier that
 *   references a Ds4TouchRegion will not execute unless
 *   its InputModifier conditions are met.
 *
 * in conclusion:
 * - it does, indirectly;
 * - (InputMapBase <- Ds4TouchRegion), (InputModifier : InputMapBase) -> (InputMap: InputMapBase)
 */

struct Ds4TouchHistory
{
	using Clock     = std::chrono::high_resolution_clock;
	using TimePoint = Clock::time_point;
	using Duration  = Clock::duration;

	TimePoint timestamp {};
	Ds4Vector2 point {};

	explicit Ds4TouchHistory(const Ds4Vector2& point)
		: timestamp(Clock::now()),
		  point(point)
	{
	}

	Ds4TouchHistory() = default;
};

/**
 * \brief A user-defined \c Ds4Device touch region.
 * \sa Ds4Device
 */
class Ds4TouchRegion : public JsonData
{
	Ds4Vector2 pointStart1 {};
	Ds4Vector2 pointStart2 {};

	/**
	 * \brief "Buttons" (touches) active in this region.
	 * \sa Ds4Buttons, Ds4Buttons_t
	 */
	Ds4Buttons_t activeButtons = 0;

	circular_buffer<Ds4TouchHistory, 30> points1 {}, points2 {};

	std::shared_ptr<TrackballSimulator> trackball;

public:
	std::shared_ptr<TrackballSettings> trackballSettings;

	/**
	 * \brief Pressed state for multi-touch point 1.
	 * \sa Pressable
	 */
	Pressable state1;

	/**
	 * \brief Pressed state for multi-touch point 2.
	 * \sa Pressable
	 */
	Pressable state2;

	ISimulator* getSimulator(InputSimulator* parent);
	void clamp(Ds4Vector2& point);

	// TODO: toggle for multi-touch press

	/**
	 * \brief The input type emulated by this touch region.
	 * \sa Ds4TouchRegionType
	 */
	Ds4TouchRegionType type = Ds4TouchRegionType::none;

	/**
	 * \brief If \c true, allows other touch regions to activate
	 * while this region is active and a touch point goes out of range.
	 */
	bool allowCrossOver = false;

	/**
	 * \brief The left bound of the touch region.
	 */
	short left = 0;

	/**
	 * \brief The top bound of the touch region.
	 */
	short top = 0;

	/**
	 * \brief The right bound of the touch region.
	 */
	short right = 0;

	/**
	 * \brief The bottom bound of the touch region.
	 */
	short bottom = 0;

	/**
	 * \brief Axis configuration for swipe directions if applicable.
	 */
	std::unordered_map<Direction_t, InputAxisOptions> touchAxisOptions;

	Ds4TouchRegion();

	/**
	 * \brief Constructs a pre-configured touch region.
	 * \param type The emulated input type of the touch region.
	 * \param left The left bound of the touch region.
	 * \param top The top bound of the touch region.
	 * \param right The right bound of the touch region.
	 * \param bottom The bottom bound of the touch region.
	 * \param allowCrossOver Allow cross-over.
	 */
	Ds4TouchRegion(Ds4TouchRegionType type, short left, short top, short right, short bottom, bool allowCrossOver = false);

	/**
	 * \brief Copy constructor.
	 * \param other The instance to copy from.
	 */
	Ds4TouchRegion(const Ds4TouchRegion& other);

	Ds4TouchRegion& operator=(const Ds4TouchRegion& other);

	/**
	 * \brief Check if a point is within the bounds of this touch region.
	 * \param sender The multi-touch sender (touch 1, touch 2).
	 * \param point The point to check.
	 * \return \c true if \a point is within the bounds of this touch region.
	 */
	bool isInRegion(Ds4Buttons_t sender, const Ds4Vector2& point);

	/**
	 * \brief Get the starting coordinates that activated this touch region.
	 * \param sender The multi-touch sender (touch 1, touch 2).
	 * \return The initial activation coordinates.
	 */
	[[nodiscard]] Ds4Vector2 getStartPoint(Ds4Buttons_t sender) const;

	/**
	 * \brief Check if a multi-touch sender is active in this touch region.
	 * \param sender The multi-touch sender (touch 1, touch 2).
	 * \return \c true if the sender is active in this region.
	 */
	[[nodiscard]] bool isActive(Ds4Buttons_t sender) const;

	/**
	 * \brief Activate the specified multi-touch senders at the given point in this touch region.
	 * \param sender The multi-touch sender (touch 1, touch 2).
	 * \param point The starting point of the activation.
	 */
	void setActive(Ds4Buttons_t sender, const Ds4Vector2& point);

	/**
	 * \brief De-activate the specified multi-touch senders in this region.
	 * \param sender The multi-touch sender (touch 1, touch 2).
	 */
	void setInactive(Ds4Buttons_t sender);

	/**
	 * \brief Get the delta of the activation point and current touch coordinates.
	 * \param sender The multi-touch sender (touch 1, touch 2).
	 * \param direction The direction of touch movement.
	 * \param point The current coordinates of the sender.
	 * \return The delta between the start point and \a point.
	 */
	[[nodiscard]] float getTouchDelta(Ds4Buttons_t sender, Direction_t direction) const;

	const decltype(points1)& getPoints(Ds4Buttons_t sender) const;

	/**
	 * \brief Get the dead zone of the specified touch direction.
	 * \param direction The direction of touch movement.
	 * \return The dead zone of the specified direction.
	 */
	float getDeadZone(Direction_t direction);

	/**
	 * \brief Apply the configured dead zone formula of a direction to an analog value.
	 * \param direction The direction of touch movement.
	 * \param analog The value to apply the dead zone formula to.
	 */
	void applyDeadZone(Direction_t direction, float& analog);

	bool operator==(const Ds4TouchRegion& other) const;
	bool operator!=(const Ds4TouchRegion& other) const;

	void readJson(const nlohmann::json& json) override;
	void writeJson(nlohmann::json& json) const override;
};
