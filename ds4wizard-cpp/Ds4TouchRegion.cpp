#include "stdafx.h"
#include "Ds4TouchRegion.h"

Ds4TouchRegion::Ds4TouchRegion(Ds4TouchRegionType type, short left, short top, short right, short bottom, bool allowCrossOver)
{
	Type           = type;
	AllowCrossOver = allowCrossOver;
	Left           = left;
	Top            = top;
	Right          = right;
	Bottom         = bottom;
}

Ds4TouchRegion::Ds4TouchRegion(const Ds4TouchRegion& other)
{
	Type             = other.Type;
	AllowCrossOver   = other.AllowCrossOver;
	Left             = other.Left;
	Top              = other.Top;
	Right            = other.Right;
	Bottom           = other.Bottom;
	TouchAxisOptions = other.TouchAxisOptions;
}

bool Ds4TouchRegion::IsInRegion(Ds4Buttons_t sender, const Ds4Vector2& point) const
{
	return IsInRegion(sender, point.X, point.Y);
}

bool Ds4TouchRegion::IsInRegion(Ds4Buttons_t sender, short x, short y) const
{
	if (x >= Left && x <= Right && y >= Top && y <= Bottom)
	{
		return true;
	}

	return !AllowCrossOver && IsActive(sender);
}

Ds4Vector2 Ds4TouchRegion::GetStartPoint(Ds4Buttons_t sender) const
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

bool Ds4TouchRegion::IsActive(Ds4Buttons_t sender) const
{
	return (Active & sender) != 0;
}

void Ds4TouchRegion::SetActive(Ds4Buttons_t sender, const Ds4Vector2& point)
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

	Active |= sender;

	if ((sender & Ds4Buttons::Touch1) != 0)
	{
		PointStart1 = point;
	}
	else if ((sender & Ds4Buttons::Touch2) != 0)
	{
		PointStart2 = point;
	}
}

void Ds4TouchRegion::SetInactive(Ds4Buttons_t sender)
{
	Active &= ~sender;

	if ((sender & Ds4Buttons::Touch1) != 0)
	{
		State1.Release();
	}

	if ((sender & Ds4Buttons::Touch2) != 0)
	{
		State2.Release();
	}
}

float Ds4TouchRegion::GetTouchDelta(Ds4Buttons_t sender, Direction_t direction, const Ds4Vector2& point) const
{
	short x = std::clamp(point.X, Left, Right);
	short y = std::clamp(point.Y, Top, Bottom);

	if (Type == Ds4TouchRegionType::StickAutoCenter)
	{
		Ds4Vector2 start = GetStartPoint(sender);

		int width  = std::max(start.X - Left, Right - start.X);
		int height = std::max(start.Y - Top, Bottom - start.Y);

		short sx = start.X;
		short sy = start.Y;

		float result;

		switch (direction)
		{
			case Direction::Up:
				result = std::abs(std::clamp(std::clamp(y - sy, -height, height) / static_cast<float>(height), -1.0f, 0.0f));
				break;
			case Direction::Down:
				result = std::clamp(std::clamp(y - sy, -height, height) / static_cast<float>(height), 0.0f, 1.0f);
				break;
			case Direction::Left:
				result = std::abs(std::clamp(std::clamp(x - sx, -width, width) / static_cast<float>(width), -1.0f, 0.0f));
				break;
			case Direction::Right:
				result = std::clamp(std::clamp(x - sx, -width, width) / static_cast<float>(width), 0.0f, 1.0f);
				break;
			default:
				throw /*new ArgumentOutOfRangeException(nameof(direction), direction, null) // TODO */;
		}

		return result;
	}
	else
	{
		x -= Left;
		y -= Top;

		int width  = Right - Left;
		int height = Bottom - Top;

		int cx = width / 2;
		int cy = height / 2;

		float result;

		switch (direction)
		{
			case Direction::Up:
				//result = std::abs(((y - cy).Clamp(-height, height) / (float)cy).Clamp(-1.0f, 0.0f));
				result = std::abs(std::clamp(std::clamp(y - cy, -height, height) / static_cast<float>(cy), -1.0f, 0.f));
				break;
			case Direction::Down:
				//result = ((y - cy).Clamp(-height, height) / (float)cy).Clamp(0.0f, 1.0f);
				result = std::clamp(std::clamp(y - cy, -height, height) / static_cast<float>(cy), 0.f, 1.f);
				break;
			case Direction::Left:
				//result = std::abs(((x - cx).Clamp(-width, width) / (float)cx).Clamp(-1.0f, 0.0f));
				result = std::abs(std::clamp(std::clamp(x - cx, -width, width) / static_cast<float>(cx), -1.0f, 0.f));
				break;
			case Direction::Right:
				//result = ((x - cx).Clamp(-width, width) / (float)cx).Clamp(0.0f, 1.0f);
				result = std::clamp(std::clamp(x - cx, -width, width) / static_cast<float>(cx), 0.f, 1.f);
				break;
			default:
				throw /*new ArgumentOutOfRangeException(nameof(direction), direction, null)*/;
		}

		return result;
	}
}

float Ds4TouchRegion::GetDeadZone(Direction_t direction)
{
	return TouchAxisOptions[direction].DeadZone;
}

void Ds4TouchRegion::ApplyDeadZone(Direction_t direction, float& analog)
{
	InputAxisOptions options = TouchAxisOptions[direction];
	options.ApplyDeadZone(analog);
}

bool Ds4TouchRegion::operator==(const Ds4TouchRegion& other) const
{
	return AllowCrossOver == other.AllowCrossOver
	       && Left == other.Left
	       && Top == other.Top
	       && Right == other.Right
	       && Bottom == other.Bottom
	       && TouchAxisOptions == other.TouchAxisOptions;
}

bool Ds4TouchRegion::operator!=(const Ds4TouchRegion& other) const
{
	return !(*this == other);
}
