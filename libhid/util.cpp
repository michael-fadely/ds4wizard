#include <iostream>

#include <Windows.h>
#include <initguid.h> // for GUID_DEVINTERFACE_USB_HUB
#include <usbiodef.h>
#include <hidsdi.h>
#include <SetupAPI.h>
#include <functional>

#include "hid_instance.h"
#include "util.h"

std::wstring hid::get_device_path(const HDEVINFO dev_info_set, SP_DEVICE_INTERFACE_DATA* interface, SP_DEVINFO_DATA* data) noexcept
{
	DWORD size = 0;

	SetupDiGetDeviceInterfaceDetail(dev_info_set, interface, nullptr, 0, &size, data);

	auto detail = static_cast<PSP_DEVICE_INTERFACE_DETAIL_DATA>(malloc(offsetof(SP_DEVICE_INTERFACE_DETAIL_DATA, DevicePath) + size + sizeof(TCHAR)));

	if (detail == nullptr)
	{
		return std::wstring();
	}

	detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

	bool success = SetupDiGetDeviceInterfaceDetail(dev_info_set, interface, detail, size, &size, data);

	std::wstring result;

	if (success)
	{
		result = std::wstring(detail->DevicePath);
	}

	free(detail);
	return result;
}

std::wstring hid::get_instance_id(const HDEVINFO dev_info_set, SP_DEVINFO_DATA* dev_info_data) noexcept
{
	std::wstring result;
	DWORD required = 0;
	SetupDiGetDeviceInstanceId(dev_info_set, dev_info_data, nullptr, 0, &required);

	if (!required)
	{
		return result;
	}

	auto buffer = new wchar_t[required];

	if (SetupDiGetDeviceInstanceId(dev_info_set, dev_info_data, buffer, required, &required))
	{
		result = std::wstring(buffer);
	}

	delete[] buffer;
	return result;
}

bool hid::enum_guid(const std::function<bool(const std::wstring& path, const std::wstring& instanceId)>& fn, const GUID& guid) noexcept
{
	const HDEVINFO dev_info = SetupDiGetClassDevs(&guid, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	if (dev_info == reinterpret_cast<HDEVINFO>(-1))
	{
		return true;
	}

	bool done = false;
	auto dev_info_data = cbsize_t<SP_DEVINFO_DATA>();

	for (size_t i = 0; SetupDiEnumDeviceInfo(dev_info, static_cast<DWORD>(i), &dev_info_data); i++)
	{
		auto interface_data = cbsize_t<SP_DEVICE_INTERFACE_DATA>();

		for (size_t j = 0; SetupDiEnumDeviceInterfaces(dev_info, &dev_info_data, &guid, static_cast<DWORD>(j), &interface_data); j++)
		{
			std::wstring path(get_device_path(dev_info, &interface_data));
			std::wstring instance_id(get_instance_id(dev_info, &dev_info_data));

			if (fn(path, instance_id))
			{
				done = true;
				break;
			}
		}

		if (done)
		{
			break;
		}
	}

	SetupDiDestroyDeviceInfoList(dev_info);
	return false;
}

void hid::enum_hid(const std::function<bool(HidInstance& instance)>& fn) noexcept
{
	GUID guid = {};
	HidD_GetHidGuid(&guid);

	auto callback = [fn](const std::wstring& path, const std::wstring& instanceId) -> bool
	{
		try
		{
			HidInstance hid(path, instanceId, true);
			return fn(hid);
		}
		catch (std::exception& ex)
		{
			std::cout << ex.what() << std::endl;
		}

		return false;
	};

	enum_guid(callback, guid);
}

void hid::enum_usb(const std::function<bool(const std::wstring& path, const std::wstring& instanceId)>& fn) noexcept
{
	enum_guid(fn, GUID_DEVINTERFACE_USB_HUB);
}
