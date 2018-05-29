#pragma once

struct Ds4Buttons
{
	enum T
	{
		Square      = 1 << 0,
		Cross       = 1 << 1,
		Circle      = 1 << 2,
		Triangle    = 1 << 3,
		L1          = 1 << 4,
		R1          = 1 << 5,
		L2          = 1 << 6,
		R2          = 1 << 7,
		Share       = 1 << 8,
		Options     = 1 << 9,
		L3          = 1 << 10,
		R3          = 1 << 11,
		PS          = 1 << 12,
		TouchButton = 1 << 13,
		Touch1      = 1 << 14,
		Touch2      = 1 << 15,
		Up          = 1 << 16,
		Down        = 1 << 17,
		Left        = 1 << 18,
		Right       = 1 << 19
	};
};
