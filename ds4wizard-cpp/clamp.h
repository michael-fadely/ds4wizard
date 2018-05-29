#pragma once

#include <algorithm>

template <typename T>
constexpr T clamp(T value, T low, T high)
{
	return std::max(std::min(value, high), low);
}
