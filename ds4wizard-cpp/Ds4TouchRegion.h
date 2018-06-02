#pragma once

#include <unordered_map>
#include "enums.h"
#include "Ds4Buttons.h"
#include "Ds4InputData.h"
#include "Pressable.h"
#include "clamp.h"
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
	/*[JsonIgnore]*/ Ds4Vector2 PointStart1;
	/*[JsonIgnore]*/ Ds4Vector2 PointStart2;

public:
	/*[JsonIgnore]*/
	Ds4Buttons::T Active /*{ get; private set; }*/;

	/*[JsonIgnore]*/ Pressable State1;
	/*[JsonIgnore]*/ Pressable State2;

	// TODO: toggle for multi-touch press

	Ds4TouchRegionType Type;

	bool  AllowCrossOver;
	short Left, Top, Right, Bottom;

	std::unordered_map<Direction::T, InputAxisOptions> TouchAxisOptions;

	Ds4TouchRegion()
	{
		// for json serialization
	}

	Ds4TouchRegion(Ds4TouchRegionType type, short left, short top, short right, short bottom, bool allowCrossOver = false)
	{
		Type           = type;
		AllowCrossOver = allowCrossOver;
		Left           = left;
		Top            = top;
		Right          = right;
		Bottom         = bottom;
	}

	Ds4TouchRegion(const Ds4TouchRegion& other)
	{
		Type             = other.Type;
		AllowCrossOver   = other.AllowCrossOver;
		Left             = other.Left;
		Top              = other.Top;
		Right            = other.Right;
		Bottom           = other.Bottom;
		TouchAxisOptions = other.TouchAxisOptions;
	}

	bool IsInRegion(Ds4Buttons::T sender, const Ds4Vector2& point) const
	{
		return IsInRegion(sender, point.X, point.Y);
	}

	bool IsInRegion(Ds4Buttons::T sender, short x, short y) const
	{
		if (x >= Left && x <= Right && y >= Top && y <= Bottom)
		{
			return true;
		}

		return !AllowCrossOver && IsActive(sender);
	}

	Ds4Vector2 GetStartPoint(Ds4Buttons::T sender) const
	{
		if ((sender & Ds4Buttons::Touch1) != 0)
		{
			return PointStart1;
		}

		if ((sender & Ds4Buttons::Touch2) != 0)
		{
			return PointStart2;
		}

		return {};
	}

	bool IsActive(Ds4Buttons::T sender) const
	{
		return (Active & sender) != 0;
	}

	void SetActive(Ds4Buttons::T sender, const Ds4Vector2& point)
	{
		if ((sender & Ds4Buttons::Touch1) != 0)
		{
			State1.Press();
		}

		if ((sender & Ds4Buttons::Touch2) != 0)
		{
			State2.Press();
		}

		if (IsActive(sender))
		{
			return;
		}

		*(uint32_t*)&Active |= sender;

		if ((sender & Ds4Buttons::Touch1) != 0)
		{
			PointStart1 = point;
		}
		else if ((sender & Ds4Buttons::Touch2) != 0)
		{
			PointStart2 = point;
		}
	}

	void SetInactive(Ds4Buttons::T sender)
	{
		*(uint32_t*)&Active &= ~sender;

		if ((sender & Ds4Buttons::Touch1) != 0)
		{
			State1.Release();
		}

		if ((sender & Ds4Buttons::Touch2) != 0)
		{
			State2.Release();
		}
	}

	float GetTouchDelta(Ds4Buttons::T sender, Direction::T direction, const Ds4Vector2& point) const
	{
		short x = clamp(point.X, Left, Right);
		short y = clamp(point.Y, Top, Bottom);

		if (Type == Ds4TouchRegionType::StickAutoCenter)
		{
			Ds4Vector2 start = GetStartPoint(sender);

			int width = std::max(start.X - Left, Right - start.X);
			int height = std::max(start.Y - Top, Bottom - start.Y);

			short sx = start.X;
			short sy = start.Y;

			float result;

			switch (direction)
			{
				case Direction::Up:
					result = std::abs(clamp(clamp(y - sy, -height, height) / (float)height, -1.0f, 0.0f));
					break;
				case Direction::Down:
					result = clamp(clamp(y - sy, -height, height) / (float)height, 0.0f, 1.0f);
					break;
				case Direction::Left:
					result = std::abs(clamp(clamp(x - sx, -width, width) / (float)width,-1.0f, 0.0f));
					break;
				case Direction::Right:
					result = clamp(clamp(x - sx, -width, width) / (float)width, 0.0f, 1.0f);
					break;
				default:
					throw /*new ArgumentOutOfRangeException(nameof(direction), direction, null)*/;
			}

			return result;
		}
		else
		{
			x -= Left;
			y -= Top;

			int width = Right - Left;
			int height = Bottom - Top;

			int cx = width / 2;
			int cy = height / 2;

			float result;

			switch (direction)
			{
				case Direction::Up:
					//result = std::abs(((y - cy).Clamp(-height, height) / (float)cy).Clamp(-1.0f, 0.0f));
					result = std::abs(clamp(clamp(y - cy, -height, height) / (float)cy, -1.0f, 0.f));
					break;
				case Direction::Down:
					//result = ((y - cy).Clamp(-height, height) / (float)cy).Clamp(0.0f, 1.0f);
					result = clamp(clamp(y - cy, -height, height) / (float)cy, 0.f, 1.f);
					break;
				case Direction::Left:
					//result = std::abs(((x - cx).Clamp(-width, width) / (float)cx).Clamp(-1.0f, 0.0f));
					result = std::abs(clamp(clamp(x - cx, -width, width) / (float)cx, -1.0f, 0.f));
					break;
				case Direction::Right:
					//result = ((x - cx).Clamp(-width, width) / (float)cx).Clamp(0.0f, 1.0f);
					result = clamp(clamp(x - cx, -width, width) / (float)cx, 0.f, 1.f);
					break;
				default:
					throw /*new ArgumentOutOfRangeException(nameof(direction), direction, null)*/;
			}

			return result;
		}
	}

	float GetDeadZone(Direction::T direction)
	{
		return TouchAxisOptions[direction].DeadZone;
	}

	void ApplyDeadZone(Direction::T direction, float& analog)
	{
		InputAxisOptions options = TouchAxisOptions[direction];
		options.ApplyDeadZone(analog);
	}

	bool operator==(const Ds4TouchRegion& other) const
	{
		return AllowCrossOver == other.AllowCrossOver
			&& Left == other.Left
			&& Top == other.Top
			&& Right == other.Right
			&& Bottom == other.Bottom
			&& TouchAxisOptions == other.TouchAxisOptions;
	}

	bool operator!=(const Ds4TouchRegion& other) const
	{
		return !(*this == other);
	}
};