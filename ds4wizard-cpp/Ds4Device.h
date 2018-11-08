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
#include "Event.h"

#include "average.h"
#include "Latency.h"

class Ds4Device
{
	bool peakedLatencyThreshold = false;
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

	Latency readLatency;
	Latency writeLatency;

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
	Event<Ds4Device> onDeviceClosed;
	Event<Ds4Device> onBatteryLevelChanged;
	Event<Ds4Device> onScpDeviceMissing;
	Event<Ds4Device> onScpDeviceOpenFailed;
	Event<Ds4Device> onScpXInputHandleFailure;
	Event<Ds4Device> onBluetoothExclusiveFailure;
	Event<Ds4Device> onBluetoothConnected;
	Event<Ds4Device> onBluetoothIdleDisconnect;
	Event<Ds4Device> onBluetoothDisconnected;
	Event<Ds4Device> onUsbExclusiveFailure;
	Event<Ds4Device> onUsbConnected;
	Event<Ds4Device, std::chrono::milliseconds> onLatencyThresholdExceeded;

	DeviceSettings settings;
	DeviceProfile profile;

	Ds4Input input {};
	Ds4Output output {};

	bool bluetoothConnected();
	bool usbConnected();
	bool connected();

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

	Latency getReadLatency();
	Latency getWriteLatency();

	void resetReadLatencyPeak();
	void resetWriteLatencyPeak();

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

public:
	void start();

private:
	// TODO: move all this shit out into an input simulator class of some kind
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
