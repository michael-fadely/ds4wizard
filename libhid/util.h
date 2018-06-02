#pragma once

#include <Windows.h>
#include <SetupAPI.h>
#include <functional>
#include "hid_instance.h"

template <typename T>
T cbsize_t()
{
	T result = {};
	result.cbSize = sizeof(T);
	return result;
}

namespace hid
{
	std::wstring get_device_path(HDEVINFO dev_info_set, SP_DEVICE_INTERFACE_DATA* interface, SP_DEVINFO_DATA* data = nullptr) noexcept;
	std::wstring get_instance_id(HDEVINFO dev_info_set, SP_DEVINFO_DATA* dev_info_data) noexcept;
	bool enum_guid(const std::function<bool(const std::wstring& path, const std::wstring& instanceId)>& fn, const GUID& guid) noexcept;
	void enum_hid(const std::function<bool(HidInstance& instance)>& fn) noexcept;
	void enum_usb(const std::function<bool(const std::wstring& path, const std::wstring& instanceId)>& fn) noexcept;
}
