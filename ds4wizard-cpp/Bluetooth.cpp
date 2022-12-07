#include "pch.h"
#include "Bluetooth.h"
#include <winioctl.h>
#include <bluetoothapis.h>
#include <bthioctl.h>
#include <hid_handle.h>
#include <span>

bool Bluetooth::disconnectDevice(const std::span<uint8_t>& macAddress)
{
	const size_t macAddressSize = macAddress.size();

	if (macAddressSize != macAddressLength)
	{
		return false;
	}

	// The buffer which will hold the reversed MAC address.
	// A length of 8 is required; otherwise DeviceIoControl fails.
	std::array<uint8_t, 8> buffer {};

	// Store reversed MAC address.
	for (size_t i = 0; i < macAddressSize; i++)
	{
		buffer[macAddressSize - 1 - i] = macAddress[i];
	}

	BLUETOOTH_FIND_RADIO_PARAMS findParams {};
	findParams.dwSize = sizeof(BLUETOOTH_FIND_RADIO_PARAMS);

	Handle phRadio(nullptr, true);

	const HBLUETOOTH_RADIO_FIND hFind = BluetoothFindFirstRadio(&findParams, &phRadio.nativeHandle);

	if (hFind == nullptr)
	{
		return false;
	}

	bool result = false;

	while (phRadio.nativeHandle != nullptr)
	{
		const bool success = DeviceIoControl(phRadio.nativeHandle, IOCTL_BTH_DISCONNECT_DEVICE,
		                                     // Input buffer.
		                                     buffer.data(), static_cast<DWORD>(buffer.size()),
		                                     // Output buffer (ignored in our case).
		                                     nullptr, 0,
		                                     // Bytes returned in output buffer (ignored in our case).
		                                     nullptr,
		                                     nullptr);

		phRadio.close();

		if (success)
		{
			result = true;
			break;
		}

		if (!BluetoothFindNextRadio(hFind, &phRadio.nativeHandle))
		{
			break;
		}
	}

	BluetoothFindRadioClose(hFind);
	return result;
}
