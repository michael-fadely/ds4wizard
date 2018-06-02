#include "stdafx.h"
#include "Bluetooth.h"
#include <winioctl.h>
#include <BluetoothAPIs.h>
#include <Bthioctl.h>

bool Bluetooth::DisconnectDevice(gsl::span<uint8_t> macAddress)
{
	bool result = false;

	std::array<uint8_t, 8> buffer {};

	for (int i = 0; i < macAddress.size(); i++)
	{
		buffer[5 - i] = macAddress[i];
	}

	BLUETOOTH_FIND_RADIO_PARAMS findParams {};
	findParams.dwSize = sizeof(BLUETOOTH_FIND_RADIO_PARAMS);

	HANDLE phRadio = nullptr;
	auto hFind   = BluetoothFindFirstRadio(&findParams, &phRadio);
	DWORD _;

	while (phRadio != nullptr)
	{
		bool success = DeviceIoControl(phRadio, IOCTL_BTH_DISCONNECT_DEVICE, buffer.data(), buffer.size(),
		                               nullptr, 0, &_, nullptr);

		CloseHandle(phRadio);

		if (success)
		{
			result = true;
			break;
		}

		if (!BluetoothFindNextRadio(hFind, &phRadio))
		{
			break;
		}
	}

	BluetoothFindRadioClose(hFind);
	return result;
}
