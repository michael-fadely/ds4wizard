#pragma once

#include <cstdint>
#include <memory>
#include <thread>

#include "DeviceSettings.h"
#include "DeviceProfile.h"
#include "hid_instance.h"
#include "Stopwatch.h"
#include "ScpDevice.h"
#include "Ds4Input.h"
#include "Ds4Output.h"

/*partial TODO*/ class Ds4Device
{
	std::mutex sync_lock;

	// Read-Only
	Stopwatch idleTime {};
	inline static Ds4Color fadeColor {};

	// Delta time (for things like mouse movement).
	// Assumes 1000 Hz virtual polling rate.
	Stopwatch deltaStopwatch {};
	float deltaTime      = 1.0f;

	int colorIndex = -1;

	// TODO
	/*readonly InputSimulator InputSimulator = new InputSimulator();
	IKeyboardSimulator      KeyboardSimulator => InputSimulator.Keyboard;
	IMouseSimulator         MouseSimulator    => InputSimulator.Mouse;*/

public:
	Stopwatch Latency;
	bool      DataReceived = false;

private:
	std::unique_ptr<std::thread> ioThread = nullptr;

	// Devices
	std::unique_ptr<ScpDevice> scpDevice = nullptr;

	std::unique_ptr<hid::HidInstance> usbDevice;
	std::unique_ptr<hid::HidInstance> bluetoothDevice;

	// Misc
	int realXInputIndex = 0;

	Ds4LightOptions activeLight;

	bool DisconnectOnIdle() const;
	DeviceIdleOptions::clock::duration IdleTimeout() const;
	bool IsIdle() const;

public:
	//event EventHandler DeviceClosed; // TODO
	//event EventHandler BatteryLevelChanged; // TODO

	DeviceSettings Settings;
	DeviceProfile  Profile;

	Ds4Input  Input {};
	Ds4Output Output {};

	std::string MacAddress; // TODO: getter
	std::string SafeMacAddress; // TODO: getter

	bool BluetoothConnected();
	bool UsbConnected();
	bool Connected();
	/// <summary>
	/// Gets the battery charge level in the range 1 to 10.
	/// </summary>
	uint8_t Battery() const;

	/// <summary>
	/// Indicates if the controller is charging.
	/// </summary>
	bool Charging() const;

	const std::string& Name() const;

	explicit Ds4Device(hid::HidInstance& device);

	void SaveSettings();

	/// <summary>
	/// Applies changes made to the device profile and opens device handles.
	/// </summary>
	void ApplyProfile();

private:
	bool ScpDeviceOpen();
	void ScpDeviceClose();
	void ReleaseAutoColor();

public:
	void OnProfileChanged(const std::string& newName);
	void Close();

private:
	void CloseBluetoothDevice();
	void DisconnectBluetooth();
	void CloseUsbDevice();
	static bool OpenDevice(hid::HidInstance& device, bool exclusive);

public:
	void OpenBluetoothDevice(hid::HidInstance& device);
	void OpenUsbDevice(hid::HidInstance& device);

private:
	void SetupBluetoothOutputBuffer() const;
	void SetupUsbOutputBuffer() const;
	void WriteUsbAsync();
	void WriteBluetooth();
	void Run();
	void ControllerThread();
	void OnDeviceClosed();

public:
	void Start();

private:
	void OnBatteryLevelChanged();
};
