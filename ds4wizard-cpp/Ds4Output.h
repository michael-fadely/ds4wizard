#pragma once
#include "Ds4Color.h"
#include <gsl/span>

class ScpDevice;

struct Ds4Output
{
	uint8_t  rightMotor    = 0;
	uint8_t  leftMotor     = 0;
	Ds4Color lightColor    = {};
	uint8_t  flashOnDur    = 0;
	uint8_t  flashOffDur   = 0;
	uint8_t  volumeLeft    = 50;
	uint8_t  volumeRight   = 50;
	uint8_t  volumeMic     = 50;
	uint8_t  volumeSpeaker = 50;

	bool update(const gsl::span<uint8_t>& buffer, int n) const;
	void fromXInput(int index, std::unique_ptr<ScpDevice>& device);
};
