#pragma once
#include <span>

/**
 * \brief Class for managing Bluetooth state.
 */
class Bluetooth
{
public:
	/**
	 * \brief Disconnects a device with matching MAC address from
	 * the first Bluetooth radio it is connected to.
	 * \param macAddress The MAC address to search for.
	 * \return \c true on success.
	 */
	static bool disconnectDevice(std::span<const uint8_t> macAddress);
};
