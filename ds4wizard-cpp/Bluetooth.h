#pragma once
#include <gsl/span>

class Bluetooth
{
public:
	/**
	 * \brief 
	 * Disconnects a device with matching MAC address from
	 * the first Bluetooth radio it is connected to.
	 * \param macAddress The MAC address to search for.
	 * \return \c true on success.
	 */
	static bool disconnectDevice(const gsl::span<uint8_t>& macAddress);
};
