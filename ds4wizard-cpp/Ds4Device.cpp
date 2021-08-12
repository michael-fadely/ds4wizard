#include "pch.h"

#include <chrono>
#include <thread>
#include <iomanip>

#include <fmt/format.h>

#include "Ds4Device.h"
#include "program.h"
#include "DeviceProfileCache.h"
#include "Bluetooth.h"
#include "Ds4AutoLightColor.h"

// TODO: allow enabling, disabling, and remapping of individual output (and eventual virtual input) DS4 motors
// TODO: allow enabling, disabling, and remapping of individual input XInput rumble motors

// TODO: !!! retry count for exclusive lock acquisition
// TODO: !!! external method for exclusive lock acquisition (i.e. to enable retry from gui)

using namespace std::chrono;

Ds4ConnectEvent::Ds4ConnectEvent(ConnectionType connectionType_, Status status_, std::optional<size_t> nativeError_)
	: connectionType(connectionType_),
	  status(status_),
	  nativeError(nativeError_)
{
}

Ds4DisconnectEvent::Ds4DisconnectEvent(ConnectionType connectionType_, Reason reason_, std::optional<size_t> nativeError_)
	: connectionType(connectionType_),
	  reason(reason_),
	  nativeError(nativeError_)
{
}

bool Ds4Device::disconnectOnIdle() const
{
	return settings.useProfileIdle ? profile.idle.disconnect : settings.idle.disconnect;
}

microseconds Ds4Device::idleTimeout() const
{
	return settings.useProfileIdle ? profile.idle.timeout : settings.idle.timeout;
}

bool Ds4Device::isIdle() const
{
	return idleTime.elapsed() >= idleTimeout();
}

bool Ds4Device::bluetoothConnected()
{
	auto lock_guard = lock();
	return bluetoothDevice != nullptr && bluetoothDevice->isOpen();
}

bool Ds4Device::usbConnected()
{
	auto lock_guard = lock();
	return usbDevice != nullptr && usbDevice->isOpen();
}

bool Ds4Device::connected()
{
	return bluetoothConnected() || usbConnected();
}

const std::string& Ds4Device::macAddress() const
{
	return macAddress_;
}

const std::string& Ds4Device::safeMacAddress() const
{
	return safeMacAddress_;
}

uint8_t Ds4Device::battery() const
{
	return input.data.battery;
}

// FIXME: This name is super misleading!
bool Ds4Device::charging() const
{
	return (input.data.extensions & Ds4Extensions::cable) != 0;
}

const std::string& Ds4Device::name() const
{
	return settings.name.empty() ? macAddress_ : settings.name;
}

Ds4Device::Ds4Device()
	: simulator(this)
{
}

Ds4Device::Ds4Device(std::shared_ptr<hid::HidInstance> device)
	: simulator(this)
{
	open(std::move(device));
}

// static
Ds4Device::MacAddress Ds4Device::getMacAddress(std::shared_ptr<hid::HidInstance> device)
{
	MacAddress mac {};

	// USB connection type
	if (device->caps().inputReportSize == usbInputReportSize)
	{
		// From DS4Windows
		std::array<uint8_t, 16> buffer {};
		buffer[0] = 18;

		if (!device->getFeature(buffer))
		{
			const std::string hidPath(device->path.begin(), device->path.end());
			throw std::runtime_error(fmt::format("Failed to read MAC address from USB device {0}", hidPath));
		}

		mac =
		{
			buffer[6], buffer[5], buffer[4],
			buffer[3], buffer[2], buffer[1]
		};
	}
	else
	{
		for (size_t i = 0; i < mac.size(); i++)
		{
			const auto sub = device->serialString.substr(i * 2, 2);
			mac[i] = static_cast<uint8_t>(std::stoul(sub, nullptr, 16));
		}
	}

	return mac;
}

void Ds4Device::open(std::shared_ptr<hid::HidInstance> device)
{
	const MacAddress macAddress = getMacAddress(device);

	macAddress_ = fmt::format("{:2X}:{:2X}:{:2X}:{:2X}:{:2X}:{:2X}",
	                          macAddress[0], macAddress[1], macAddress[2],
	                          macAddress[3], macAddress[4], macAddress[5]);

	safeMacAddress_ = fmt::format("{:2x}{:2x}{:2x}{:2x}{:2x}{:2x}",
	                              macAddress[0], macAddress[1], macAddress[2],
	                              macAddress[3], macAddress[4], macAddress[5]);

	if (!!memcmp(macAddressBytes.data(), macAddress.data(), macAddress.size()))
	{
		macAddressBytes = macAddress;
	}

	if (device->caps().inputReportSize != usbInputReportSize)
	{
		bluetoothDevice = std::move(device);
		setupBluetoothOutputBuffer();
	}
	else
	{
		// HACK: this member shouldn't be exposed, but getting the "serial" (MAC) over USB is non-standard for the DS4.
		device->serialString = fmt::format(L"{:2x}{:2x}{:2x}{:2x}{:2x}{:2x}",
		                                   macAddress[0], macAddress[1], macAddress[2],
		                                   macAddress[3], macAddress[4], macAddress[5]);

		usbDevice = std::move(device);
		setupUsbOutputBuffer();
	}

	std::optional<DeviceSettings> cachedSettings = Program::profileCache.getSettings(macAddress_);

	if (!cachedSettings.has_value())
	{
		this->settings = {};
	}
	else
	{
		this->settings = *cachedSettings;
	}

	notifiedLow = false;
	notifiedCharged = true;

	applyProfile();
}

void Ds4Device::saveSettings()
{
	auto lock_guard = lock();
	Program::profileCache.saveSettings(macAddress_, settings);
}

void Ds4Device::applySettings(const DeviceSettings& newSettings)
{
	auto lock_guard = lock();
	settings = newSettings;
	saveSettings();
	applyProfile();
}

void Ds4Device::applyProfile()
{
	auto lock_guard = lock();
	releaseAutoColor();

	std::optional<DeviceProfile> cachedProfile = Program::profileCache.getProfile(settings.profile);

	if (!cachedProfile.has_value())
	{
		settings.profile = {};
		profile = DeviceProfile::defaultProfile();
	}
	else
	{
		profile = *cachedProfile;
	}

	Ds4LightOptions& lightOptions = settings.useProfileLight ? profile.light : settings.light;

	if (lightOptions.automaticColor)
	{
		lightOptions.color = Ds4AutoLightColor::getColor(colorIndex);
	}

	activeLight = lightOptions;

	if (usbDevice != nullptr && (!usbConnected() || usbDevice->isExclusive() != profile.exclusiveMode))
	{
		closeUsbDevice();
		std::shared_ptr<hid::HidInstance> instance = std::move(usbDevice);
		openUsbDevice(instance);
	}

	if (bluetoothDevice != nullptr && (!bluetoothConnected() || bluetoothDevice->isExclusive() != profile.exclusiveMode))
	{
		closeBluetoothDevice();
		std::shared_ptr<hid::HidInstance> instance = std::move(bluetoothDevice);
		openBluetoothDevice(instance);
	}

	if (this->connected())
	{
		simulator.applyProfile(&profile);
	}

	idleTime.start();
}

void Ds4Device::releaseAutoColor()
{
	auto lock_guard = lock();
	Ds4AutoLightColor::releaseColor(colorIndex);
	colorIndex = -1;
}

void Ds4Device::onProfileChanged(const std::string& newName)
{
	auto lock_guard = lock();
	settings.profile = newName.empty() ? std::string() : newName;
	saveSettings();
	applyProfile();
}

Ds4Device::~Ds4Device()
{
	try
	{
		close();
	}
	catch (std::exception&)
	{
		// ignored
	}
}

std::unique_lock<std::recursive_mutex> Ds4Device::lock()
{
	return std::unique_lock<std::recursive_mutex>(sync_lock);
}

Latency Ds4Device::getReadLatency()
{
	auto lock_guard = lock();
	return readLatency;
}

Latency Ds4Device::getWriteLatency()
{
	auto lock_guard = lock();
	return writeLatency;
}

void Ds4Device::resetReadLatencyPeak()
{
	auto lock_guard = lock();
	readLatency.resetPeak();
}

void Ds4Device::resetWriteLatencyPeak()
{
	auto lock_guard = lock();
	writeLatency.resetPeak();
}

void Ds4Device::closeImpl()
{
	auto lock_guard = lock();
	running = false;

	closeUsbDevice();
	closeBluetoothDevice();

	releaseAutoColor();
}

void Ds4Device::close()
{
	running = false;

	if (deviceThread && deviceThread->get_id() == std::this_thread::get_id())
	{
		deviceThread->detach();
		deviceThread = nullptr;
	}

	if (!deviceThread)
	{
		closeImpl();
		return;
	}

	deviceThread->join();
	deviceThread = nullptr;
}

void Ds4Device::closeBluetoothDevice()
{
	auto lock_guard = lock();

	if (bluetoothDevice != nullptr && bluetoothDevice->isOpen())
	{
		bluetoothDevice->close();
	}

	idleTime.start();
}

void Ds4Device::disconnectBluetooth(BluetoothDisconnectReason reason)
{
	if (!bluetoothConnected())
	{
		return;
	}

	for (size_t i = 0; !Bluetooth::disconnectDevice(macAddressBytes) && i < 5; i++)
	{
		std::this_thread::sleep_for(125ms);
	}

	closeBluetoothDevice();

	const auto eventReason = reason == BluetoothDisconnectReason::idle ? Ds4DisconnectEvent::Reason::idle : Ds4DisconnectEvent::Reason::closed;
	onDisconnect.invoke(this, Ds4DisconnectEvent(ConnectionType::bluetooth, eventReason));
}

void Ds4Device::closeUsbDevice()
{
	auto lock_guard = lock();

	if (usbDevice != nullptr && usbDevice->isOpen())
	{
		usbDevice->close();
	}

	idleTime.start();
}

bool Ds4Device::openDevice(std::shared_ptr<hid::HidInstance>& hid, bool exclusive)
{
	if (hid->open((exclusive ? hid::HidOpenFlags::exclusive : 0) | hid::HidOpenFlags::async))
	{
		return true;
	}

	return exclusive && hid->open(hid::HidOpenFlags::async);
}

bool Ds4Device::openBluetoothDevice(std::shared_ptr<hid::HidInstance> hid)
{
	auto lock_guard = lock();

	if (bluetoothConnected())
	{
		return true;
	}

	if (!openDevice(hid, profile.exclusiveMode))
	{
		onConnectFailure.invoke(this, Ds4ConnectEvent(ConnectionType::bluetooth, Ds4ConnectEvent::Status::openFailed, hid->nativeError()));
		return false;
	}

	auto isExclusiveFailure = [&]() -> bool { return profile.exclusiveMode && !hid->isExclusive(); };
	const bool wasExclusiveFailure = isExclusiveFailure();

	if (wasExclusiveFailure)
	{
		hid->close();

		try
		{
			Ds4DeviceManager::toggleDevice(hid->instanceId);
		}
		catch (const std::exception&)
		{
			onConnectFailure.invoke(this, Ds4ConnectEvent(ConnectionType::bluetooth,
			                                              Ds4ConnectEvent::Status::toggleFailed,
			                                              GetLastError()));

			return false;
		}

		if (!openDevice(hid, profile.exclusiveMode))
		{
			onConnectFailure.invoke(this, Ds4ConnectEvent(ConnectionType::bluetooth, Ds4ConnectEvent::Status::openFailed, hid->nativeError()));
			return false;
		}
	}

	// Enables bluetooth operational mode which makes
	// the controller send report id 17 (0x11)
	std::array<uint8_t, 37> temp {};
	temp[0] = 0x02;

	// TODO: on failure, pull the error code from GetLastError internally instead of doing it manually
	if (!hid->getFeature(temp))
	{
		hid->close();
		onWirelessOperationalModeFailure.invoke(this, GetLastError());
		return false;
	}

	if (wasExclusiveFailure && isExclusiveFailure())
	{
		// exclusive failure is non-critical
		onConnect.invoke(this, Ds4ConnectEvent(ConnectionType::bluetooth, Ds4ConnectEvent::Status::exclusiveFailed));
	}
	else
	{
		onConnect.invoke(this, Ds4ConnectEvent(ConnectionType::bluetooth, Ds4ConnectEvent::Status::opened));
	}

	bluetoothDevice = std::move(hid);

	setupBluetoothOutputBuffer();
	idleTime.start();
	return true;
}

bool Ds4Device::openUsbDevice(std::shared_ptr<hid::HidInstance> hid)
{
	auto lock_guard = lock();

	if (usbConnected())
	{
		return true;
	}

	if (!openDevice(hid, profile.exclusiveMode))
	{
		onConnectFailure.invoke(this, Ds4ConnectEvent(ConnectionType::usb, Ds4ConnectEvent::Status::openFailed, hid->nativeError()));
		return false;
	}

	auto isExclusiveFailure = [&]() -> bool { return profile.exclusiveMode && !hid->isExclusive(); };
	const bool wasExclusiveFailure = isExclusiveFailure();

	if (wasExclusiveFailure)
	{
		hid->close();

		try
		{
			Ds4DeviceManager::toggleDevice(hid->instanceId);
		}
		catch (const std::exception&)
		{
			onConnectFailure.invoke(this, Ds4ConnectEvent(ConnectionType::usb,
			                                              Ds4ConnectEvent::Status::toggleFailed,
			                                              GetLastError()));

			return false;
		}

		if (!openDevice(hid, profile.exclusiveMode))
		{
			onConnectFailure.invoke(this, Ds4ConnectEvent(ConnectionType::usb, Ds4ConnectEvent::Status::openFailed, hid->nativeError()));
			return false;
		}
	}

	if (wasExclusiveFailure && isExclusiveFailure())
	{
		// exclusive failure is non-critical
		onConnect.invoke(this, Ds4ConnectEvent(ConnectionType::usb, Ds4ConnectEvent::Status::exclusiveFailed));
	}
	else
	{
		onConnect.invoke(this, Ds4ConnectEvent(ConnectionType::usb, Ds4ConnectEvent::Status::opened));
	}

	usbDevice = std::move(hid);
	setupUsbOutputBuffer();
	return true;
}

void Ds4Device::setupBluetoothOutputBuffer() const
{
	bluetoothDevice->outputBuffer[0] = 0x11;
	bluetoothDevice->outputBuffer[1] = 0x80; // For HID + CRC, use 0xC0.
	bluetoothDevice->outputBuffer[3] = 0x0F;
}

void Ds4Device::setupUsbOutputBuffer() const
{
	usbDevice->outputBuffer[0] = 0x05;
	usbDevice->outputBuffer[1] = 0xFF;
}

void Ds4Device::writeUsbAsync()
{
	// FIXME: This + async io rewrite prevents detection of device removal.
	// If the controller is unplugged during an async write, Windows I/O
	// polling doesn't detect a failure. Although it appears to be caught
	// in the read phase now. Worth investigating.
	if (usbDevice->asyncWriteInProgress())
	{
		return;
	}

	writeLatency.stop();

	constexpr auto usb_output_offset = 4;

	const auto span = gsl::span(&usbDevice->outputBuffer[usb_output_offset],
	                            usbDevice->outputBuffer.size() - usb_output_offset);

	if (!output.update(span))
	{
		writeTime.start();
		return;
	}

	if (!usbDevice->writeAsync())
	{
		usbDevice->close();
	}

	writeLatency.start();
	writeTime.start();
}

void Ds4Device::writeBluetooth()
{
	constexpr auto bt_output_offset = 6;

	const auto span = gsl::span(&bluetoothDevice->outputBuffer[bt_output_offset],
	                            bluetoothDevice->outputBuffer.size() - bt_output_offset);

	if (!output.update(span))
	{
		writeTime.start();
		return;
	}

	writeLatency.start();

	if (!bluetoothDevice->setOutputReport())
	{
		closeBluetoothDevice();
	}

	writeLatency.stop();
	writeTime.start();
}

void Ds4Device::run()
{
	// HACK: make this class manage the light state
	output.lightColor = activeLight.color;

	// HACK: see above
	if (activeLight.idleFade)
	{
		const Ds4LightOptions& l = settings.useProfileLight ? profile.light : settings.light;
		const double m = isIdle() ? 1.0 : std::clamp(duration_cast<milliseconds>(idleTime.elapsed()).count()
		                                             / static_cast<double>(duration_cast<milliseconds>(idleTimeout()).count()),
		                                             0.0, 1.0);

		output.lightColor = Ds4Color::lerp(l.color, fadeColor, static_cast<float>(m));
	}

	const bool lastChargingState = charging();
	const uint8_t lastBatteryLevel = battery();

	// cache
	const bool usb = usbConnected();
	const bool bluetooth = bluetoothConnected();

	const ConnectionType preferredConnection = Program::settings.preferredConnection;
	const bool useUsb = usb && (preferredConnection == +ConnectionType::usb || !bluetooth);
	const bool useBluetooth = bluetooth && (preferredConnection == +ConnectionType::bluetooth || !usb);

	dataReceived = false;

	auto asyncRead = [](hid::HidInstance* i) -> bool
	{
		if (!i->isOpen())
		{
			return false;
		}

		if (i->asyncReadPending())
		{
			return !i->asyncReadInProgress();
		}

		if (!i->readAsync())
		{
			i->close();
			return false;
		}

		return !i->asyncReadInProgress();
	};

	if (useUsb)
	{
		writeUsbAsync();

		if (asyncRead(usbDevice.get()))
		{
			dataReceived = true;

			constexpr auto usb_input_offset = 1;
			const auto span = gsl::span(&usbDevice->inputBuffer[usb_input_offset],
			                            usbDevice->inputBuffer.size() - usb_input_offset);

			input.update(span);
		}

		// If the controller gets disconnected from USB while idle,
		// reset the idle timer so that it doesn't get immediately
		// disconnected from bluetooth (if connected).
		if (!usbConnected())
		{
			// FIXME: Use of nativeError() is unlikely to be reliable in simultaneous async read/write because both update it.
			const auto reason = usbDevice->nativeError() != ERROR_DEVICE_NOT_CONNECTED
			                    ? Ds4DisconnectEvent::Reason::error
			                    : Ds4DisconnectEvent::Reason::closed;

			onDisconnect.invoke(this, Ds4DisconnectEvent(ConnectionType::usb, reason, usbDevice->nativeError()));

			closeUsbDevice();
			idleTime.start();
		}
	}
	else if (useBluetooth)
	{
		writeBluetooth();

		if (asyncRead(bluetoothDevice.get()) && bluetoothDevice->inputBuffer[0] == 0x11)
		{
			dataReceived = true;

			constexpr auto bt_input_offset = 3;
			const auto span = gsl::span(&bluetoothDevice->inputBuffer[bt_input_offset],
			                            bluetoothDevice->inputBuffer.size() - bt_input_offset);

			input.update(span);
		}
	}

	const float lx = input.getAxis(Ds4Axes::leftStickX, std::nullopt);
	const float ly = input.getAxis(Ds4Axes::leftStickY, std::nullopt);
	const float ls = std::sqrt(lx * lx + ly * ly);

	const float rx = input.getAxis(Ds4Axes::rightStickX, std::nullopt);
	const float ry = input.getAxis(Ds4Axes::rightStickY, std::nullopt);
	const float rs = std::sqrt(rx * rx + ry * ry);

	// TODO: gyro/accel - definitely needs to be configurable
	if (input.buttonsChanged || input.heldButtons ||
	    ls >= 0.25f || rs >= 0.25f)
	{
		idleTime.start();
	}
	else if (disconnectOnIdle() && useBluetooth && !charging() && isIdle())
	{
		disconnectBluetooth(BluetoothDisconnectReason::idle);
	}

	if (dataReceived)
	{
		simulator.runMaps();
		readLatency.stop();

		const auto average = duration_cast<milliseconds>(readLatency.average());

		if (average > settings.latencyThreshold)
		{
			if (!peakedLatencyThreshold)
			{
				// do the thing
				peakedLatencyThreshold = true;
				onLatencyThresholdExceeded.invoke(this, average, settings.latencyThreshold);
			}
		}
		else
		{
			peakedLatencyThreshold = false;
		}

		readLatency.start();

		if (lastChargingState != charging() || lastBatteryLevel != battery())
		{
			if (settings.notifyBatteryLow > 0)
			{
				if (usbConnected() || charging() || battery() > settings.notifyBatteryLow)
				{
					notifiedLow = false;
				}
				else if (!notifiedLow)
				{
					notifiedLow = true;
					onBatteryLevelLow.invoke(this, battery());
				}
			}

			if (settings.notifyFullyCharged)
			{
				if ((!usbConnected() && !charging()) || battery() < 10)
				{
					notifiedCharged = false;
				}
				else if (!notifiedCharged && battery() >= 10)
				{
					notifiedCharged = true;
					onBatteryFullyCharged.invoke(this);
				}
			}

			onBatteryLevelChanged.invoke(this);
		}

#if false
		// Experimental garbage
		if (input.pressedButtons & Ds4Buttons::ps)
		{
			// Set audio output to speaker only (3)
			std::array<uint8_t, 3> buffer = {
				0xE0,
				0,
				3
			};

			usbDevice->setFeature(buffer);

			std::array<uint8_t, 0x20> volume {};
			// Report ID
			volume[0x00] = 5;
			// Audio endpoints
			volume[0x01] = 0x10 | 0x20 | 0x80; // Jack L, R, Speaker
			// Jack L
			volume[0x13] = 0;
			// Jack R
			volume[0x14] = 0;
			// Speaker
			volume[0x16] = 0;

			usbDevice->write(volume);
		}
#endif
	}
	else
	{
		input.updateChangedState();
		simulator.runPersistent();
	}
}

void Ds4Device::controllerThread()
{
	simulator.start();
	readLatency.start();
	idleTime.start();
	writeTime.start();

	while (connected() && running)
	{
		{
			auto lock_guard = lock();
			run();
		}

		if (!dataReceived)
		{
			std::this_thread::sleep_for(1ms);
		}
	}

	closeImpl();
	onDeviceClose.invoke(this);
}

void Ds4Device::start()
{
	if (deviceThread == nullptr)
	{
		running = true;
		deviceThread = std::make_unique<std::thread>(&Ds4Device::controllerThread, this);
	}
}
