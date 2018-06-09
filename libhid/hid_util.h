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
	bool enumerateGUID(const std::function<bool(const std::wstring& path, const std::wstring& instanceId)>& fn, const GUID& guid) noexcept;
	void enumerateHID(const std::function<bool(HidInstance& instance)>& fn) noexcept;
	void enumerateUSB(const std::function<bool(const std::wstring& path, const std::wstring& instanceId)>& fn) noexcept;
}
