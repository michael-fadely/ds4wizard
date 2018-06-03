#pragma once

#include <array>
#include <mutex>

#include "Ds4Color.h"

class Ds4AutoLightColor
{
	struct Pair
	{
		const Ds4Color Color;
		int References = 0;

		Pair(const Ds4Color& color);
	};

	inline static std::recursive_mutex sync_lock;

	static std::array<Pair, 17> colors;

public:
	static Ds4Color GetColor(int& index);
	static void ReleaseColor(int index);
};