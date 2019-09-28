#pragma once
#include <Xinput.h>
#include <cstdint>

/**
 * \brief Simple wrapper for \c XINPUT_GAMEPAD with comparison operators and method for outputting bytes.
 */
struct XInputGamepad : XINPUT_GAMEPAD
{
	bool operator==(const XInputGamepad& r) const;
	bool operator!=(const XInputGamepad& r) const;

	void toBytes(uint8_t* buffer, int offset) const;
};
