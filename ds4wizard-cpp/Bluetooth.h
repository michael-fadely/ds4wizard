#pragma once
#include <gsl/span>

class Bluetooth
{
public:
	/// <summary>
	/// Disconnects a device with matching MAC address from
	/// the first Bluetooth radio it is connected to.
	/// </summary>
	/// <param name="macAddress">The MAC address to search for.</param>
	/// <returns><value>true</value> on success.</returns>
	static bool DisconnectDevice(const gsl::span<uint8_t>& macAddress);
};
