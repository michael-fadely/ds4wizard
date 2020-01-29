#include <Windows.h>
#include <initguid.h> // for GUID_DEVINTERFACE_USB_HUB
#include <usbiodef.h>
#include <hidsdi.h>
#include <SetupAPI.h>
#include <functional>

#include "hid_instance.h"
#include "hid_util.h"

std::wstring hid::getDevicePath(const HDEVINFO devInfoSet, SP_DEVICE_INTERFACE_DATA* interface, SP_DEVINFO_DATA* data) noexcept
{
	DWORD size = 0;

	SetupDiGetDeviceInterfaceDetail(devInfoSet, interface, nullptr, 0, &size, data);

	auto detail = static_cast<PSP_DEVICE_INTERFACE_DETAIL_DATA>(malloc(offsetof(SP_DEVICE_INTERFACE_DETAIL_DATA, DevicePath) + size + sizeof(TCHAR)));

	if (detail == nullptr)
	{
		return std::wstring();
	}

	detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

	bool success = SetupDiGetDeviceInterfaceDetail(devInfoSet, interface, detail, size, &size, data);

	std::wstring result;

	if (success)
	{
		result = std::wstring(detail->DevicePath);
	}

	free(detail);
	return result;
}

std::wstring hid::getInstanceId(const HDEVINFO devInfoSet, SP_DEVINFO_DATA* devInfoData) noexcept
{
	std::wstring result;
	DWORD required = 0;
	SetupDiGetDeviceInstanceId(devInfoSet, devInfoData, nullptr, 0, &required);

	if (!required)
	{
		return result;
	}

	auto buffer = new wchar_t[required];

	if (SetupDiGetDeviceInstanceId(devInfoSet, devInfoData, buffer, required, &required))
	{
		result = std::wstring(buffer);
	}

	delete[] buffer;
	return result;
}

bool hid::enumerateGuid(const std::function<bool(const std::wstring& path, const std::wstring& instanceId)>& fn, const GUID& guid) noexcept
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
			const std::wstring path(getDevicePath(dev_info, &interface_data));
			const std::wstring instance_id(getInstanceId(dev_info, &dev_info_data));

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

void hid::enumerateHid(const std::function<bool(std::shared_ptr<HidInstance> instance)>& fn) noexcept
{
	GUID guid = {};
	HidD_GetHidGuid(&guid);

	const auto callback = [fn](const std::wstring& path, const std::wstring& instanceId) -> bool
	{
		auto hid = std::make_shared<HidInstance>(path, instanceId);

		if (hid->readMetadata())
		{
			return fn(hid);
		}

		return false;
	};

	enumerateGuid(callback, guid);
}

void hid::enumerateUsb(const std::function<bool(const std::wstring& path, const std::wstring& instanceId)>& fn) noexcept
{
	enumerateGuid(fn, GUID_DEVINTERFACE_USB_HUB);
}
