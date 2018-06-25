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
	Stopwatch latency;
	bool dataReceived = false;

private:
	std::unique_ptr<std::thread> deviceThread = nullptr;

	// Devices
	std::unique_ptr<ScpDevice> scpDevice = nullptr;

	std::unique_ptr<hid::HidInstance> usbDevice;
	std::unique_ptr<hid::HidInstance> bluetoothDevice;

	// Misc
	int realXInputIndex = 0;

	Ds4LightOptions activeLight;

	bool disconnectOnIdle() const;
	std::chrono::milliseconds idleTimeout() const;
	bool isIdle() const;

public:
	EventHandler<EventArgs> deviceClosed;
	EventHandler<EventArgs> batteryLevelChanged;

	DeviceSettings settings;
	DeviceProfile profile;

	Ds4Input input {};
	Ds4Output output {};

	std::string macAddress; // TODO: getter
	std::string safeMacAddress; // TODO: getter

	bool bluetoothConnected();
	bool usbConnected();
	bool connected();
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

	explicit Ds4Device(hid::HidInstance& device);
	~Ds4Device();

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
	void onDeviceClosed();

public:
	void start();

private:
	void onBatteryLevelChanged();

#pragma region shit
	static constexpr Ds4Buttons_t touchMask = Ds4Buttons::touch1 | Ds4Buttons::touch2;

	void simulateXInputButton(XInputButtons_t buttons, PressedState state);

	XInputAxis_t simulatedXInputAxis = 0;

	void simulateXInputAxis(XInputAxes& axes, float m);
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
