#include "stdafx.h"
#include "ScpDevice.h"
#include "Ds4Output.h"

bool Ds4Output::Update(const gsl::span<uint8_t>& buffer, int n) const
{
	int i = n;

	bool result = buffer[i] != RightMotor;
	buffer[i++] = RightMotor;

	result      = result || buffer[i] != LeftMotor;
	buffer[i++] = LeftMotor;

	result      = result || buffer[i] != LightColor.Red;
	buffer[i++] = LightColor.Red;

	result      = result || buffer[i] != LightColor.Green;
	buffer[i++] = LightColor.Green;

	result      = result || buffer[i] != LightColor.Blue;
	buffer[i++] = LightColor.Blue;

	result      = result || buffer[i] != FlashOnDur;
	buffer[i++] = FlashOnDur;

	result    = result || buffer[i] != FlashOffDur;
	buffer[i] = FlashOffDur;

	i = n + 17;

	result      = result || buffer[i] != VolumeLeft;
	buffer[i++] = VolumeLeft;

	result      = result || buffer[i] != VolumeRight;
	buffer[i++] = VolumeRight;

	result      = result || buffer[i] != VolumeMic;
	buffer[i++] = VolumeMic;

	result    = result || buffer[i] != VolumeSpeaker;
	buffer[i] = VolumeSpeaker;

	return result;
}

void Ds4Output::FromXInput(int index, std::unique_ptr<ScpDevice>& device)
{
	device->GetVibration(index, LeftMotor, RightMotor);
}
