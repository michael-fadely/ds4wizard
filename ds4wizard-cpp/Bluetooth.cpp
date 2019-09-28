#include "pch.h"
#include "Bluetooth.h"
#include <winioctl.h>
#include <BluetoothAPIs.h>
#include <Bthioctl.h>
#include <hid_handle.h>

bool Bluetooth::disconnectDevice(const gsl::span<uint8_t>& macAddress)
{
	bool result = false;

	std::array<uint8_t, 8> buffer {};

	for (ptrdiff_t i = 0; i < macAddress.size(); i++)
	{
		buffer[5 - i] = macAddress[i];
	}

	BLUETOOTH_FIND_RADIO_PARAMS findParams {};
	findParams.dwSize = sizeof(BLUETOOTH_FIND_RADIO_PARAMS);

	Handle phRadio(nullptr, true);

	auto hFind = BluetoothFindFirstRadio(&findParams, &phRadio.nativeHandle);

	if (hFind == nullptr)
	{
		return false;
	}

	DWORD _;

	while (phRadio.nativeHandle != nullptr)
	{
		bool success = DeviceIoControl(phRadio.nativeHandle, IOCTL_BTH_DISCONNECT_DEVICE,
		                               buffer.data(), static_cast<DWORD>(buffer.size()), nullptr, 0, &_, nullptr);

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
