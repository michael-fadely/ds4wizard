#pragma once

#include <cstdint>
#include <memory>
#include <thread>
#include <functional>
#include <deque>

#include "DeviceSettings.h"
#include "DeviceProfile.h"
#include "hid_instance.h"
#include "Stopwatch.h"
#include "ScpDevice.h"
#include "Ds4Input.h"
#include "Ds4Output.h"

class Ds4Device
{
	// HACK: work around for what may or may not be a bug with recursive_mutex;
	// the main thread fails to acquire a lock when the mutex is UNLOCKED,
	// and I suspect it might be due to async file IO that is used with hid.
	bool running = false;
	std::recursive_mutex sync_lock;

	std::deque<Ds4TouchRegion*> touchRegions;

	// Read-Only
	Stopwatch idleTime {};
	inline static Ds4Color fadeColor {};

	// Delta time (for things like mouse movement).
	// Assumes 1000 Hz virtual polling rate.
	Stopwatch deltaStopwatch {};
	float deltaTime = 1.0f;

	int colorIndex = -1;

	// TODO
	/*readonly InputSimulator InputSimulator = new InputSimulator();
	IKeyboardSimulator      KeyboardSimulator => InputSimulator.Keyboard;
	IMouseSimulator         MouseSimulator    => InputSimulator.Mouse;*/

public:
	Stopwatch Latency;
	bool DataReceived = false;

private:
	std::unique_ptr<std::thread> deviceThread = nullptr;

	// Devices
	std::unique_ptr<ScpDevice> scpDevice = nullptr;

	std::unique_ptr<hid::HidInstance> usbDevice;
	std::unique_ptr<hid::HidInstance> bluetoothDevice;

	// Misc
	int realXInputIndex = 0;

	Ds4LightOptions activeLight;

	bool DisconnectOnIdle() const;
	std::chrono::nanoseconds IdleTimeout() const;
	bool IsIdle() const;

public:
	//event EventHandler DeviceClosed; // TODO
	//event EventHandler BatteryLevelChanged; // TODO

	DeviceSettings Settings;
	DeviceProfile Profile;

	Ds4Input Input {};
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
	~Ds4Device();

private:
	void closeImpl();

public:
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

#pragma region shit
	static constexpr Ds4Buttons_t touchMask = Ds4Buttons::touch1 | Ds4Buttons::touch2;

	void SimulateXInputButton(XInputButtons_t buttons, PressedState state);

	XInputAxis_t simulatedXInputAxis = 0;

	void SimulateXInputAxis(XInputAxes& axes, float m);
	bool IsOverriddenByModifierSet(InputMapBase& map);
	void RunMap(InputMap& m, InputModifier* modifier);
	static PressedState HandleTouchToggle(InputMap& m, InputModifier* modifier, const Pressable& pressable);
	void ApplyMap(InputMap& m, InputModifier* modifier, PressedState state, float analog);
	void SimulateMouse(const InputMap& m, PressedState state, float analog);
	void SimulateKeyboard(const InputMap& m, PressedState state);
	void RunAction(ActionType action);
	void RunMaps();
	void RunPersistent();
	void UpdateTouchRegions();
	void UpdateTouchRegion(Ds4TouchRegion& region, InputModifier* modifier, Ds4Buttons_t sender, Ds4Vector2& point, Ds4Buttons_t& disallow);
	void UpdatePressedStateImpl(InputMapBase& instance, const std::function<void()>& press, const std::function<void()>& release);
	void UpdatePressedState(InputModifier& modifier);
	void UpdatePressedState(InputMap& map, InputModifier* modifier);
	void UpdateBindingState(InputMap& m, InputModifier* modifier);
#pragma endregion
};
