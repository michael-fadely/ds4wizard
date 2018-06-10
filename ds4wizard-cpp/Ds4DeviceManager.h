#pragma once

#include <array>
#include <map>
#include <mutex>
#include <string>

#include <hid_instance.h>
#include "Ds4Device.h"
#include "EventHandler.h"

class DeviceOpenedEventArgs
{
public:
	/// <summary>
	/// The device that triggered the event.
	/// </summary>
	const std::shared_ptr<Ds4Device> Device;

		/// <summary>
		/// <value>true</value> if this is the first connection for this
		/// device, <value>false</value> if an additional connection has
		/// been added to this device.
		/// </summary>
	const bool Unique;

	DeviceOpenedEventArgs(std::shared_ptr<Ds4Device> device, bool unique)
		: Device(std::move(device)),
		  Unique(unique)
	{
	}
};

class DeviceClosedEventArgs
{
public:
	/// <summary>
	/// The device that triggered the event.
	/// </summary>
	const std::shared_ptr<Ds4Device> Device;

	explicit DeviceClosedEventArgs(std::shared_ptr<Ds4Device> device)
		: Device(std::move(device))
	{
	}
};

class Ds4DeviceManager
{
	std::recursive_mutex sync_lock, devices_lock;
	std::map<std::wstring, std::shared_ptr<Ds4Device>> devices;

public:
	/// <summary>
	/// Defines the DualShock 4 Vendor ID.
	/// </summary>
	static const short vendorId = 0x54c;

	/// <summary>
	/// Defines validated DualShock 4 Product IDs.
	/// </summary>
	inline static const std::array<short, 3> productIds = { 0xba0, 0x5c4, 0x9cc };

	/// <summary>
	/// Fired when a device is opened.
	/// </summary>
	/// <seealso cref="DeviceOpenedEventArgs"/>
	EventHandler<DeviceOpenedEventArgs> deviceOpened;

	/// <summary>
	/// Fired when a device is closed.
	/// </summary>
	/// <seealso cref="DeviceClosedEventArgs"/>
	EventHandler<DeviceClosedEventArgs> deviceClosed;

	~Ds4DeviceManager();

	/* TODO
	IEnumerable<Ds4Device> Enumerate()
	{
		lock(devices)
		return devices ? .Select(x = > x.Value);
	}
	*/

	/// <summary>
	/// Checks if a device is a DualShock 4.
	/// </summary>
	/// <param name="hid">The HID instance to be checked.</param>
	/// <returns><value>true</value> if this device is a DualShock 4.</returns>
	static bool isDs4(const hid::HidInstance& hid);

	/// <summary>
	/// Checks if a device is a DualShock 4.
	/// </summary>
	/// <param name="devicePath">The path to the device to be checked.</param>
	/// <returns><value>true</value> if this device is a DualShock 4.</returns>
	static bool isDs4(const std::wstring& devicePath);

	/// <summary>
	/// Discovers and connects to all DualShock 4 devices.
	/// </summary>
	void findControllers();

private:
	void onDs4DeviceClosed(void* sender, EventArgs* eventArgs);

public:
	/// <summary>
	/// Closes and removes all managed devices.
	/// </summary>
	void close();

private:
	static void queueDeviceToggle(const std::wstring& instanceId);

public:
	static void toggleDeviceElevated(const std::wstring& instanceId);

private:
	static void toggleDevice(const std::wstring& instanceId);

	void onDeviceOpened(DeviceOpenedEventArgs& e);
	void onDeviceClosed(DeviceClosedEventArgs& e);
};
