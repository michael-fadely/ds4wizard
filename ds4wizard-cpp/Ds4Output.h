#pragma once
#include "Ds4Color.h"
#include <gsl/span>

class ScpDevice;

struct Ds4Output
{
	uint8_t  RightMotor    = 0;
	uint8_t  LeftMotor     = 0;
	Ds4Color LightColor    = {};
	uint8_t  FlashOnDur    = 0;
	uint8_t  FlashOffDur   = 0;
	uint8_t  VolumeLeft    = 50;
	uint8_t  VolumeRight   = 50;
	uint8_t  VolumeMic     = 50;
	uint8_t  VolumeSpeaker = 50;

	bool Update(const gsl::span<uint8_t>& buffer, int n) const;
	void FromXInput(int index, std::unique_ptr<ScpDevice>& device);
};
