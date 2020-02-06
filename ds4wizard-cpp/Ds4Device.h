#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <thread>

#include "DeviceSettings.h"
#include "DeviceProfile.h"
#include "hid_instance.h"
#include "Stopwatch.h"
#include "Ds4Input.h"
#include "Ds4Output.h"
#include "Event.h"

#include "Latency.h"
#include "InputSimulator.h"

class Ds4ConnectEvent
{
public:
	enum class Status
	{
		opened,
		toggleFailed,
		exclusiveFailed,
		openFailed
	};

	ConnectionType connectionType;
	Status status;
	std::optional<size_t> nativeError;

	Ds4ConnectEvent() = delete;

	Ds4ConnectEvent(ConnectionType connectionType_, Status status_, std::optional<size_t> nativeError_ = std::nullopt);
};

class Ds4DisconnectEvent
{
public:
	enum class Reason
	{
		dropped,
		closed,
		idle,
		error
	};

	ConnectionType connectionType;
	Reason reason;
	std::optional<size_t> nativeError;

	Ds4DisconnectEvent() = delete;

	Ds4DisconnectEvent(ConnectionType connectionType_, Reason reason_, std::optional<size_t> nativeError_ = std::nullopt);
};

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

	Stopwatch idleTime {};
	Stopwatch writeTime {};
	const std::chrono::milliseconds writeFrequency = std::chrono::milliseconds(4);

	inline static const Ds4Color fadeColor {};

	ptrdiff_t colorIndex = -1;

	Latency readLatency;
	Latency writeLatency;

	bool dataReceived = false;

	std::unique_ptr<std::thread> deviceThread = nullptr;

	std::shared_ptr<hid::HidInstance> usbDevice;
	std::shared_ptr<hid::HidInstance> bluetoothDevice;

	Ds4LightOptions activeLight;

	bool disconnectOnIdle() const;
	std::chrono::microseconds idleTimeout() const;
	bool isIdle() const;

	InputSimulator simulator;

public:
	enum class BluetoothDisconnectReason
	{
		none,
		idle
	};

	Event<Ds4Device> onDeviceClose;
	Event<Ds4Device> onBatteryLevelChanged;
	Event<Ds4Device, Ds4ConnectEvent> onConnect;
	Event<Ds4Device, size_t> onWirelessOperationalModeFailure;
	Event<Ds4Device, Ds4ConnectEvent> onConnectFailure;
	Event<Ds4Device, Ds4DisconnectEvent> onDisconnect;

	// value, threshold
	Event<Ds4Device, std::chrono::milliseconds, std::chrono::milliseconds> onLatencyThresholdExceeded;

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
	 * \brief Gets the battery charge level in the range 1 to 10.
	 */
	uint8_t battery() const;

	/**
	 * \brief Indicates if the controller is charging.
	 */
	bool charging() const;

	const std::string& name() const;

	Ds4Device();
	explicit Ds4Device(std::shared_ptr<hid::HidInstance> device);
	~Ds4Device();

	void open(std::shared_ptr<hid::HidInstance> device);

	std::unique_lock<std::recursive_mutex> lock();

	Latency getReadLatency();
	Latency getWriteLatency();

	void resetReadLatencyPeak();
	void resetWriteLatencyPeak();

private:
	void closeImpl();

public:
	void saveSettings();
	void applySettings(const DeviceSettings& newSettings);

	/**
	 * \brief Applies changes made to the device profile and opens device handles.
	 */
	void applyProfile();

private:
	void releaseAutoColor();

public:
	void onProfileChanged(const std::string& newName);
	void close();

	void closeBluetoothDevice();

	void disconnectBluetooth(BluetoothDisconnectReason reason);

	void closeUsbDevice();
	static bool openDevice(std::shared_ptr<hid::HidInstance>& hid, bool exclusive);

public:
	bool openBluetoothDevice(std::shared_ptr<hid::HidInstance> hid);
	bool openUsbDevice(std::shared_ptr<hid::HidInstance> hid);

private:
	void setupBluetoothOutputBuffer() const;
	void setupUsbOutputBuffer() const;
	void writeUsbAsync();
	void writeBluetooth();
	void run();
	void controllerThread();

public:
	void start();
};
