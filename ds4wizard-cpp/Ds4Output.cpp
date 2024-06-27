#include "pch.h"
#include "Ds4Output.h"

bool Ds4Output::update(std::span<uint8_t> buffer) const
{
	size_t i = 0;

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

	result      = result || buffer[i] != flashOffDur;
	buffer[i]   = flashOffDur;

	i = 17;

	result      = result || buffer[i] != volumeLeft;
	buffer[i++] = volumeLeft;

	result      = result || buffer[i] != volumeRight;
	buffer[i++] = volumeRight;

	result      = result || buffer[i] != volumeMic;
	buffer[i++] = volumeMic;

	result      = result || buffer[i] != volumeSpeaker;
	buffer[i]   = volumeSpeaker;

	return result;
}
