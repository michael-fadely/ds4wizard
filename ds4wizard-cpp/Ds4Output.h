#pragma once
#include "Ds4Color.h"
#include <span>

/**
 * \brief PoD for setting \c Ds4Device parameters like motor speed, light color, etc.
 */
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

	/**
	 * \brief Updates a buffer with the stored device parameters.
	 * \param buffer Buffer to update.
	 * \return \c true if changes have been made to \a buffer.
	 */
	bool update(const std::span<uint8_t>& buffer) const;
};
