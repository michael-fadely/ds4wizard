#pragma once

#include <unordered_map>

#include <enum.h>

#include "enums.h"
#include "Ds4InputData.h"
#include "Pressable.h"
#include "AxisOptions.h"
#include "JsonData.h"

// TODO: TrackPad
// TODO: TrackBall (surface friction, constant friction, "haptic" feedback)
// TODO: Slider
// TODO: Make auto-center an option instead of type?

BETTER_ENUM(Ds4TouchRegionType, int, none, button, stick, stickAutoCenter)

class Ds4TouchRegion : public JsonData
{
	/*[JsonIgnore]*/
	Ds4Vector2 pointStart1 {};
	/*[JsonIgnore]*/
	Ds4Vector2 pointStart2 {};

public:
	/*[JsonIgnore]*/
	Ds4Buttons_t activeButtons; // TODO: private set

	/*[JsonIgnore]*/
	Pressable state1;
	/*[JsonIgnore]*/
	Pressable state2;

	// TODO: toggle for multi-touch press

	Ds4TouchRegionType type = Ds4TouchRegionType::none;

	bool allowCrossOver = false;
	short left          = 0;
	short top           = 0;
	short right         = 0;
	short bottom        = 0;

	std::unordered_map<Direction_t, InputAxisOptions> touchAxisOptions;

	Ds4TouchRegion() = default;
	Ds4TouchRegion(Ds4TouchRegionType type, short left, short top, short right, short bottom, bool allowCrossOver = false);
	Ds4TouchRegion(const Ds4TouchRegion& other);

	bool isInRegion(Ds4Buttons_t sender, const Ds4Vector2& point) const;
	bool isInRegion(Ds4Buttons_t sender, short x, short y) const;
	Ds4Vector2 getStartPoint(Ds4Buttons_t sender) const;
	bool isActive(Ds4Buttons_t sender) const;
	void setActive(Ds4Buttons_t sender, const Ds4Vector2& point);
	void setInactive(Ds4Buttons_t sender);
	float getTouchDelta(Ds4Buttons_t sender, Direction_t direction, const Ds4Vector2& point) const;
	float getDeadZone(Direction_t direction);
	void applyDeadZone(Direction_t direction, float& analog);
	bool operator==(const Ds4TouchRegion& other) const;
	bool operator!=(const Ds4TouchRegion& other) const;
	void readJson(const QJsonObject& json) override;
	void writeJson(QJsonObject& json) const override;
};
