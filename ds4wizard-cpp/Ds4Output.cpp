#include "stdafx.h"
#include "ScpDevice.h"
#include "Ds4Output.h"

// TODO: just offset buffer by n
bool Ds4Output::update(const gsl::span<uint8_t>& buffer, size_t n) const
{
	size_t i = n;

	bool result = buffer[i] != rightMotor;
	buffer[i++] = rightMotor;

	result      = result || buffer[i] != leftMotor;
	buffer[i++] = leftMotor;

	result      = result || buffer[i] != lightColor.red;
	buffer[i++] = lightColor.red;

	result      = result || buffer[i] != lightColor.green;
	buffer[i++] = lightColor.green;

	result      = result || buffer[i] != lightColor.blue;
	buffer[i++] = lightColor.blue;

	result      = result || buffer[i] != flashOnDur;
	buffer[i++] = flashOnDur;

	result    = result || buffer[i] != flashOffDur;
	buffer[i] = flashOffDur;

	i = n + 17;

	result      = result || buffer[i] != volumeLeft;
	buffer[i++] = volumeLeft;

	result      = result || buffer[i] != volumeRight;
	buffer[i++] = volumeRight;

	result      = result || buffer[i] != volumeMic;
	buffer[i++] = volumeMic;

	result    = result || buffer[i] != volumeSpeaker;
	buffer[i] = volumeSpeaker;

	return result;
}

void Ds4Output::fromXInput(int index, std::unique_ptr<ScpDevice>& device)
{
	device->getVibration(index, leftMotor, rightMotor);
}
