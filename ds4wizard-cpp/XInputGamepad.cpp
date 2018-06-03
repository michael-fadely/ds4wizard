#include "stdafx.h"
#include "XInputGamepad.h"

bool XInputGamepad::operator==(const XInputGamepad& r) const
{
	return wButtons == r.wButtons
	       && bLeftTrigger == r.bLeftTrigger
	       && bRightTrigger == r.bRightTrigger
	       && sThumbLX == r.sThumbLX
	       && sThumbLY == r.sThumbLY
	       && sThumbRX == r.sThumbRX
	       && sThumbRY == r.sThumbRY;
}

bool XInputGamepad::operator!=(const XInputGamepad& r) const
{
	return !(*this == r);
}

void XInputGamepad::toBytes(uint8_t* buffer, int offset) const
{
	buffer[offset + 0] = (uint8_t)((int)wButtons & 0xFF);
	buffer[offset + 1] = (uint8_t)(((int)wButtons >> 8) & 0xFF);
	buffer[offset + 2] = bLeftTrigger;
	buffer[offset + 3] = bRightTrigger;

	buffer[offset + 4] = (uint8_t)(sThumbLX & 0xFF);
	buffer[offset + 5] = (uint8_t)((sThumbLX >> 8) & 0xFF);

	buffer[offset + 6] = (uint8_t)(sThumbLY & 0xFF);
	buffer[offset + 7] = (uint8_t)((sThumbLY >> 8) & 0xFF);

	buffer[offset + 8] = (uint8_t)(sThumbRX & 0xFF);
	buffer[offset + 9] = (uint8_t)((sThumbRX >> 8) & 0xFF);

	buffer[offset + 10] = (uint8_t)(sThumbRY & 0xFF);
	buffer[offset + 11] = (uint8_t)((sThumbRY >> 8) & 0xFF);
}
