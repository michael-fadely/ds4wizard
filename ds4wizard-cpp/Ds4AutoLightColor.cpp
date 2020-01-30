#include "pch.h"
#include "Ds4AutoLightColor.h"
#include "lock.h"

std::array<Ds4AutoLightColor::Pair, 17> Ds4AutoLightColor::colors =
{
	Pair(Ds4Color(0,   0,   64)),
	Pair(Ds4Color(64,  0,   0)),
	Pair(Ds4Color(0,   64,  0)),
	Pair(Ds4Color(32,  0,   32)),
	Pair(Ds4Color(12,  23,  29)),
	Pair(Ds4Color(0,   86,  0)),
	Pair(Ds4Color(105, 0,   98)),
	Pair(Ds4Color(0,   114, 0)),
	Pair(Ds4Color(97,  0,   116)),
	Pair(Ds4Color(0,   105, 0)),
	Pair(Ds4Color(111, 0,   110)),
	Pair(Ds4Color(0,   32,  0)),
	Pair(Ds4Color(84,  0,   105)),
	Pair(Ds4Color(0,   109, 0)),
	Pair(Ds4Color(101, 0,   111)),
	Pair(Ds4Color(0,   117, 0)),
	Pair(Ds4Color(116, 0,   0))
};

Ds4AutoLightColor::Pair::Pair(const Ds4Color& color)
	: color(color)
{
}

Ds4Color Ds4AutoLightColor::getColor(ptrdiff_t& index)
{
	MAKE_GUARD(lock);

	ptrdiff_t minIndex = 0;
	ptrdiff_t refCount = std::numeric_limits<ptrdiff_t>::max();

	for (size_t i = 0; i < colors.size(); i++)
	{
		const Pair& c = colors[i];

		if (c.references == 0)
		{
			minIndex = i;
			break;
		}

		if (c.references < refCount)
		{
			minIndex = i;
			refCount = c.references;
		}
	}

	Pair& pair = colors[minIndex];
	++pair.references;
	index = minIndex;
	return Ds4Color(pair.color);
}

void Ds4AutoLightColor::releaseColor(ptrdiff_t index)
{
	if (index < 0)
	{
		return;
	}

	MAKE_GUARD(lock);

	Pair& pair = colors[index];

	if (pair.references > 0)
	{
		--pair.references;
	}
}
