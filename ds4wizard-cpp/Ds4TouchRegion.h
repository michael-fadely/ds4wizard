#pragma once

#include <unordered_map>
#include "enums.h"
#include "Ds4Buttons.h"
#include "Ds4InputData.h"
#include "Pressable.h"
#include "AxisOptions.h"

enum class Ds4TouchRegionType
{
	// TODO: TrackPad
	// TODO: TrackBall (surface friction, constant friction, "haptic" feedback)
	// TODO: Slider
	// TODO: Make auto-center an option instead of type?

	None,
	Button,
	Stick,
	StickAutoCenter
};

class Ds4TouchRegion
{
	/*[JsonIgnore]*/ Ds4Vector2 PointStart1 {};
	/*[JsonIgnore]*/ Ds4Vector2 PointStart2 {};

public:
	/*[JsonIgnore]*/
	Ds4Buttons_t Active = 0; // TODO: private set

	/*[JsonIgnore]*/ Pressable State1;
	/*[JsonIgnore]*/ Pressable State2;

	// TODO: toggle for multi-touch press

	Ds4TouchRegionType Type = Ds4TouchRegionType::None;

	bool  AllowCrossOver = false;
	short Left           = 0;
	short Top            = 0;
	short Right          = 0;
	short Bottom         = 0;

	std::unordered_map<Direction_t, InputAxisOptions> TouchAxisOptions;

	Ds4TouchRegion() = default;
	Ds4TouchRegion(Ds4TouchRegionType type, short left, short top, short right, short bottom, bool allowCrossOver = false);
	Ds4TouchRegion(const Ds4TouchRegion& other);

	bool IsInRegion(Ds4Buttons_t sender, const Ds4Vector2& point) const;
	bool IsInRegion(Ds4Buttons_t sender, short x, short y) const;
	Ds4Vector2 GetStartPoint(Ds4Buttons_t sender) const;
	bool IsActive(Ds4Buttons_t sender) const;
	void SetActive(Ds4Buttons_t sender, const Ds4Vector2& point);
	void SetInactive(Ds4Buttons_t sender);
	float GetTouchDelta(Ds4Buttons_t sender, Direction_t direction, const Ds4Vector2& point) const;
	float GetDeadZone(Direction_t direction);
	void ApplyDeadZone(Direction_t direction, float& analog);
	bool operator==(const Ds4TouchRegion& other) const;
	bool operator!=(const Ds4TouchRegion& other) const;
};