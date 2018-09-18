#pragma once

#include <chrono>
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
#include "EventHandler.h"

class Ds4Device
{
	std::string macAddress_;
	std::string safeMacAddress_;

	// HACK: work around for what may or may not be a bug with recursive_mutex;
	// the main thread fails to acquire a lock when the mutex is UNLOCKED,
	// and I suspect it might be due to async file IO that is used with hid.
	bool running = false;
	std::recursive_mutex sync_lock;

	std::deque<Ds4TouchRegion*> touchRegions;

	// Read-Only
	Stopwatch idleTime {};

	inline static const Ds4Color fadeColor {};

	// Delta time (for things like mouse movement).
	// Assumes 1000 Hz virtual polling rate.
	Stopwatch deltaStopwatch {};
	float deltaTime = 1.0f;

	ptrdiff_t colorIndex = -1;

	// TODO
	/*readonly InputSimulator InputSimulator = new InputSimulator();
	IKeyboardSimulator      KeyboardSimulator => InputSimulator.Keyboard;
	IMouseSimulator         MouseSimulator    => InputSimulator.Mouse;*/

	Stopwatch latency;

	size_t latencyPoints = 0;
	Stopwatch::clock::duration latencySum = std::chrono::milliseconds(0);
	// used for external reading
	Stopwatch::clock::duration storedLatency = std::chrono::milliseconds(0);
	Stopwatch::clock::duration peakLatency = std::chrono::milliseconds(0);

	bool dataReceived = false;

	std::unique_ptr<std::thread> deviceThread = nullptr;

	// Devices
	std::unique_ptr<ScpDevice> scpDevice = nullptr;

	std::unique_ptr<hid::HidInstance> usbDevice;
	std::unique_ptr<hid::HidInstance> bluetoothDevice;

	// Misc
	int realXInputIndex = 0;

	Ds4LightOptions activeLight;

	bool disconnectOnIdle() const;
	std::chrono::microseconds idleTimeout() const;
	bool isIdle() const;

public:
	EventHandler<Ds4Device, EventArgs> onDeviceClosed;
	EventHandler<Ds4Device, EventArgs> onBatteryLevelChanged;
	EventHandler<Ds4Device, EventArgs> onScpDeviceMissing;          // Logger::writeLine(LogLevel::warning, "ScpVBus device not found. XInput emulation will not be available.");
	EventHandler<Ds4Device, EventArgs> onScpDeviceOpenFailed;       // Logger::writeLine(LogLevel::warning, "Failed to acquire ScpVBus device handle. XInput emulation will not be available.");
	EventHandler<Ds4Device, EventArgs> onScpXInputHandleFailure;    // Logger::writeLine(LogLevel::warning, "Failed to obtain ScpVBus XInput handle. XInput emulation will not be available.");
	EventHandler<Ds4Device, EventArgs> onBluetoothExclusiveFailure; // Logger::writeLine(LogLevel::warning, name(), "Failed to open Bluetooth device exclusively.");
	EventHandler<Ds4Device, EventArgs> onBluetoothConnected;        // Logger::writeLine(LogLevel::info, name(), "Bluetooth connected.");
	EventHandler<Ds4Device, EventArgs> onBluetoothIdleDisconnect;   // Logger::writeLine(LogLevel::info, name(), "Bluetooth idle disconnect." /* TODO: std::string.Format(Resources.IdleDisconnect, idleTimeout)*/);
	EventHandler<Ds4Device, EventArgs> onBluetoothDisconnected;     // Logger::writeLine(LogLevel::info, name(), "Bluetooth disconnected.");
	EventHandler<Ds4Device, EventArgs> onUsbExclusiveFailure;       // Logger::writeLine(LogLevel::warning, name(), "Failed to open USB device exclusively.");
	EventHandler<Ds4Device, EventArgs> onUsbConnected;              // Logger::writeLine(LogLevel::info, name(), "USB connected.");

	DeviceSettings settings;
	DeviceProfile profile;

	Ds4Input input {};
	Ds4Output output {};

	bool bluetoothConnected();
	bool usbConnected();
	bool connected();

	Stopwatch::clock::duration getLatency();
	Stopwatch::clock::duration getLatencyAverage();
	Stopwatch::clock::duration getLatencyPeak();
	void resetLatencyPeak();

	const std::string& macAddress() const;
	const std::string& safeMacAddress() const;

	/**
	 * \brief 
	 * Gets the battery charge level in the range 1 to 10.
	 */
	uint8_t battery() const;

	/**
	 * \brief 
	 * Indicates if the controller is charging.
	 */
	bool charging() const;

	const std::string& name() const;

	Ds4Device() = default;
	explicit Ds4Device(hid::HidInstance& device);
	~Ds4Device();

	void open(hid::HidInstance& device);

	std::unique_lock<std::recursive_mutex> lock();

private:
	void closeImpl();

public:
	void saveSettings();

	/**
	 * \brief 
	 * Applies changes made to the device profile and opens device handles.
	 */
	void applyProfile();

private:
	bool scpDeviceOpen();
	void scpDeviceClose();
	void releaseAutoColor();

public:
	void onProfileChanged(const std::string& newName);
	void close();

private:
	void closeBluetoothDevice();
	void disconnectBluetooth();
	void closeUsbDevice();
	static bool openDevice(hid::HidInstance& device, bool exclusive);

public:
	void openBluetoothDevice(hid::HidInstance& device);
	void openUsbDevice(hid::HidInstance& device);

private:
	void setupBluetoothOutputBuffer() const;
	void setupUsbOutputBuffer() const;
	void writeUsbAsync();
	void writeBluetooth();
	void run();
	void controllerThread();
	void addLatencySum();

public:
	void start();

private:
#pragma region shit
	static constexpr Ds4Buttons_t touchMask = Ds4Buttons::touch1 | Ds4Buttons::touch2;

	void simulateXInputButton(XInputButtons_t buttons, PressedState state);

	XInputAxis_t simulatedXInputAxis = 0;

	void simulateXInputAxis(XInputAxes& axes, float m);

	// TODO: OPTIMIZE !!!
	std::deque<InputMap*> __maps;
	void getActive(InputMapBase& map);

	bool isOverriddenByModifierSet(InputMapBase& map);
	void runMap(InputMap& m, InputModifier* modifier);
	static PressedState handleTouchToggle(InputMap& m, InputModifier* modifier, const Pressable& pressable);
	void applyMap(InputMap& m, InputModifier* modifier, PressedState state, float analog);
	void simulateMouse(const InputMap& m, PressedState state, float analog);
	void simulateKeyboard(const InputMap& m, PressedState state);
	void runAction(ActionType action);
	void runMaps();
	void runPersistent();
	void updateTouchRegions();
	void updateTouchRegion(Ds4TouchRegion& region, InputModifier* modifier, Ds4Buttons_t sender, Ds4Vector2& point, Ds4Buttons_t& disallow);
	void updatePressedStateImpl(InputMapBase& instance, const std::function<void()>& press, const std::function<void()>& release);
	void updatePressedState(InputModifier& modifier);
	void updatePressedState(InputMap& map, InputModifier* modifier);
	void updateBindingState(InputMap& m, InputModifier* modifier);
#pragma endregion
};
