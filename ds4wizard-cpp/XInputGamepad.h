#pragma once
#include <Xinput.h>
#include <cstdint>

struct XInputButtons
{
	enum T : uint16_t
	{
		DPadUp        = 0x0001,
		DPadDown      = 0x0002,
		DPadLeft      = 0x0004,
		DPadRight     = 0x0008,
		Start         = 0x0010,
		Back          = 0x0020,
		LeftThumb     = 0x0040,
		RightThumb    = 0x0080,
		LeftShoulder  = 0x0100,
		RightShoulder = 0x0200,
		Guide         = 0x0400,
		Dummy         = 0x0800,
		A             = 0x1000,
		B             = 0x2000,
		X             = 0x4000,
		Y             = 0x8000,
	};
};

using XInputButtons_t = uint16_t;

struct XInputGamepad : XINPUT_GAMEPAD
{
	bool operator==(const XInputGamepad& r) const;
	bool operator!=(const XInputGamepad& r) const;

	void toBytes(uint8_t* buffer, int offset) const;
};
