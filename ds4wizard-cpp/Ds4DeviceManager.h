#pragma once

#include <array>
#include <map>
#include <mutex>
#include <string>

#include <hid_instance.h>
#include "Ds4Device.h"

/* TODO
class DeviceOpenedEventArgs
{
	/// <summary>
	/// The device that triggered the event.
	/// </summary>
	public Ds4Device Device { get; }

		/// <summary>
		/// <value>true</value> if this is the first connection for this
		/// device, <value>false</value> if an additional connection has
		/// been added to this device.
		/// </summary>
	public bool Unique { get; }

		public DeviceOpenedEventArgs(Ds4Device device, bool unique)
	{
		Device = device;
		Unique = unique;
	}
}

public class DeviceClosedEventArgs
{
	/// <summary>
	/// The device that triggered the event.
	/// </summary>
	public Ds4Device Device { get; }

		public DeviceClosedEventArgs(Ds4Device device)
	{
		Device = device;
	}
}
*/

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
	//event EventHandler<DeviceOpenedEventArgs> DeviceOpened; // TODO

	/// <summary>
	/// Fired when a device is closed.
	/// </summary>
	/// <seealso cref="DeviceClosedEventArgs"/>
	//event EventHandler<DeviceClosedEventArgs> DeviceClosed; // TODO

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
	/* TODO: void OnDs4DeviceClosed(object sender, EventArgs eventArgs)
	{
		if (!(sender is Ds4Device device))
		{
			return;
		}

		lock(devices);
		OnDeviceClosed(new DeviceClosedEventArgs(device));
		devices.Remove(device.SafeMacAddress);
	}*/

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

	/* TODO: void OnDeviceOpened(DeviceOpenedEventArgs e)
	{
		DeviceOpened?.Invoke(this, e);
	}*/

	/* TODO: void OnDeviceClosed(DeviceClosedEventArgs e)
	{
		DeviceClosed?.Invoke(this, e);
	}*/
};
