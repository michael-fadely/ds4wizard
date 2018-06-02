#pragma once

#include <array>
#include <map>
#include <mutex>
#include <string>
#include <iomanip>
#include <sstream>
#include <utility>

#include <Shellapi.h>
#include <SetupAPI.h>
#include <hidsdi.h>
#include <hidpi.h>

#include <hid_instance.h>

#include "program.h"
#include "util.h"

#define lock(NAME) std::lock_guard<std::mutex> NAME ## _guard(NAME ## _lock)

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
	std::mutex sync_lock, devices_lock;
	std::map<std::wstring, std::shared_ptr<Ds4Device>> devices;

public:
	/// <summary>
	/// Defines the DualShock 4 Vendor ID.
	/// </summary>
	static const short Ds4VendorID = 0x54c;

	/// <summary>
	/// Defines validated DualShock 4 Product IDs.
	/// </summary>
	inline static const std::array<short, 3> Ds4ProductIDs = { 0xba0, 0x5c4, 0x9cc };


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
	static bool IsDs4(const hid::HidInstance& hid)
	{
		if (hid.attributes().vendor_id == Ds4VendorID)
		{
			for (auto id : Ds4ProductIDs)
			{
				if (id == hid.attributes().product_id)
				{
					return true;
				}
			}
		}

		return false;
	}

	/// <summary>
	/// Checks if a device is a DualShock 4.
	/// </summary>
	/// <param name="devicePath">The path to the device to be checked.</param>
	/// <returns><value>true</value> if this device is a DualShock 4.</returns>
	static bool IsDs4(const std::wstring& devicePath)
	{
		const hid::HidInstance hid(devicePath, true);
		return IsDs4(hid);
	}

	/// <summary>
	/// Discovers and connects to all DualShock 4 devices.
	/// </summary>
	void FindControllers()
	{
		lock(sync);

		hid::enum_hid([&](hid::HidInstance& hid) -> void
		{
			bool isBluetooth = false;

			try
			{
				isBluetooth = hid.caps().input_report_size != 64;

				// USB connection type
				if (!isBluetooth)
				{
					// From DS4Windows
					std::array<uint8_t, 16> buffer {};
					buffer[0] = 18;

					if (!hid.get_feature(buffer))
					{
						throw /*std::runtime_error(std::wstring.Format(Resources.DeviceReadMACFailed, hid.path)) // TODO */;
					}

					hid.serial =
					{
						buffer[6], buffer[5], buffer[4],
						buffer[3], buffer[2], buffer[1]
					};

					std::wstringstream serialString;

					serialString << std::setw(2) << std::setfill('0') << std::hex
						<< buffer[6] << buffer[5] << buffer[4] << buffer[3] << buffer[2] << buffer[1];

					hid.serial_string = serialString.str();
				}

				if (hid.serial_string.empty())
				{
					throw /*std::runtime_error(std::wstring.Format(Resources.DeviceReturnedEmptyMAC, hid.Path)) // TODO */;
				}
			}
			catch (const std::runtime_error& ex)
			{
				// TODO: proper HID exceptions
				//Logger.WriteLine(LogLevel.Warning, $"Failed to read device metadata: {ex.Message}"); // TODO
				hid.close();
				return;
			}

			try
			{
				lock(devices);

				const auto it = devices.find(hid.serial_string);
				std::shared_ptr<Ds4Device> device = nullptr;

				if (it != devices.end())
				{
					QueueDeviceToggle(hid.instance_id);
					device = std::make_shared<Ds4Device>(hid);
					//device.DeviceClosed += OnDs4DeviceClosed; // TODO

					//OnDeviceOpened(new DeviceOpenedEventArgs(device, true)); // TODO
					device.Start();

					devices[device.SafeMacAddress] = device;
				}
				else
				{
					if (isBluetooth)
					{
						if (!device.BluetoothConnected)
						{
							QueueDeviceToggle(hid.instance_id);
							device.OpenBluetoothDevice(hid);
						}
					}
					else
					{
						if (!device.UsbConnected)
						{
							QueueDeviceToggle(hid.instance_id);
							device.OpenUsbDevice(hid);
						}
					}

					//OnDeviceOpened(new DeviceOpenedEventArgs(device, false)); // TODO
				}
			}
			catch (const std::runtime_error& ex)
			{
				// TODO: proper HID exceptions
				//Logger.WriteLine(LogLevel.Error, $"Error while opening device: {ex.Message}"); // TODO
			}
		});
	}

private:
	/*void OnDs4DeviceClosed(object sender, EventArgs eventArgs) TODO
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
	void Close()
	{
		lock(devices);

		for (auto& pair : devices)
		{
			pair.second->Close();
		}

		devices.clear();
	}

private:
	static void QueueDeviceToggle(const std::wstring& instanceId)
	{
		ToggleDeviceElevated(instanceId);
	}

public:
	static void ToggleDeviceElevated(const std::wstring& instanceId)
	{
		if (Program::isElevated())
		{
			ToggleDevice(instanceId);
			return;
		}

		/*string currentProcess = Assembly.GetEntryAssembly().Location;

		var info = new ProcessStartInfo(currentProcess)
		{
			UseShellExecute = true,
			Verb = "runas",
			Arguments = $"--toggle-device \"{instanceId}\""
		};

		using (Process p = Process.Start(info))
		{
			p?.WaitForExit();
		}*/

		std::wstring currentProcess = QCoreApplication::applicationFilePath().toStdWString();
		std::wstring params = L"--toggle-device ";
		params.append(instanceId);

		HINSTANCE hinst = ShellExecuteW(nullptr, L"runas",
		                                currentProcess.c_str(), params.c_str(), nullptr, SW_SHOWDEFAULT);

		WaitForSingleObject(hinst, INFINITE);
	}

private:
	static void ToggleDevice(const std::wstring& instanceId);

	/*void OnDeviceOpened(DeviceOpenedEventArgs e) TODO
	{
		DeviceOpened?.Invoke(this, e);
	}*/

	/*void OnDeviceClosed(DeviceClosedEventArgs e) TODO
	{
		DeviceClosed?.Invoke(this, e);
	}*/
};
