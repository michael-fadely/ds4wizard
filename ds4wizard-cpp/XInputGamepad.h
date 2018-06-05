#pragma once
#include <Xinput.h>
#include <cstdint>

struct XInputGamepad : XINPUT_GAMEPAD
{
	bool operator==(const XInputGamepad& r) const;
	bool operator!=(const XInputGamepad& r) const;

	void toBytes(uint8_t* buffer, int offset) const;
};
