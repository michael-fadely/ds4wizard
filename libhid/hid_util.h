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
	std::wstring getDevicePath(HDEVINFO devInfoSet, SP_DEVICE_INTERFACE_DATA* interface, SP_DEVINFO_DATA* data = nullptr) noexcept;
	std::wstring getInstanceId(HDEVINFO devInfoSet, SP_DEVINFO_DATA* devInfoData) noexcept;
	bool enumerateGuid(const std::function<bool(const std::wstring& path, const std::wstring& instanceId)>& fn, const GUID& guid) noexcept;
	void enumerateHid(const std::function<bool(std::shared_ptr<HidInstance> instance)>& fn) noexcept;
	void enumerateUsb(const std::function<bool(const std::wstring& path, const std::wstring& instanceId)>& fn) noexcept;
}
