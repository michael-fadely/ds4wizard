#include "pch.h"

#include <chrono>
#include <format>
#include <thread>

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
MacAddress Ds4Device::getMacAddress(const std::shared_ptr<hid::HidInstance>& device)
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
			throw std::runtime_error(std::format("Failed to read MAC address from USB device {0}", hidPath));
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

	macAddress_ = std::format("{:2X}:{:2X}:{:2X}:{:2X}:{:2X}:{:2X}",
	                          macAddress[0], macAddress[1], macAddress[2],
	                          macAddress[3], macAddress[4], macAddress[5]);

	safeMacAddress_ = std::format("{:2x}{:2x}{:2x}{:2x}{:2x}{:2x}",
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
		device->serialString = std::format(L"{:2x}{:2x}{:2x}{:2x}{:2x}{:2x}",
		                                   macAddress[0], macAddress[1], macAddress[2],
		                                   macAddress[3], macAddress[4], macAddress[5]);

		usbDevice = std::move(device);
		setupUsbOutputBuffer();
	}

	const std::optional<DeviceSettings> cachedSettings = Program::profileCache.getSettings(macAddress_);

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

	const std::optional<DeviceProfile> cachedProfile = Program::profileCache.getProfile(settings.profile);

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
		closeDeviceAndResetIdle(usbDevice);
		openUsbDevice(std::move(usbDevice));
	}

	if (bluetoothDevice != nullptr && (!bluetoothConnected() || bluetoothDevice->isExclusive() != profile.exclusiveMode))
	{
		closeDeviceAndResetIdle(bluetoothDevice);
		openBluetoothDevice(std::move(bluetoothDevice));
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

void Ds4Device::closeDeviceAndResetIdle(const std::shared_ptr<hid::HidInstance>& device)
{
	auto lock_guard = lock();

	if (device != nullptr && device->isOpen())
	{
		device->close();
	}

	idleTime.start();
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
	return std::unique_lock(sync_lock);
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

	closeDeviceAndResetIdle(usbDevice);
	closeDeviceAndResetIdle(bluetoothDevice);

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

	closeDeviceAndResetIdle(bluetoothDevice);

	const auto eventReason = reason == BluetoothDisconnectReason::idle ? Ds4DisconnectEvent::Reason::idle : Ds4DisconnectEvent::Reason::closed;
	onDisconnect.invoke(this, Ds4DisconnectEvent(ConnectionType::bluetooth, eventReason));
}

bool Ds4Device::openDevice(const std::shared_ptr<hid::HidInstance>& hid, bool exclusive)
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

	if (!hid->getFeature(temp))
	{
		hid->close();
		onWirelessOperationalModeFailure.invoke(this, hid->nativeError());
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

	constexpr auto usbOutputOffset = 4;

	const auto span = std::span(&usbDevice->outputBuffer[usbOutputOffset],
	                            usbDevice->outputBuffer.size() - usbOutputOffset);

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
	constexpr auto btOutputOffset = 6;

	const auto span = std::span(&bluetoothDevice->outputBuffer[btOutputOffset],
	                            bluetoothDevice->outputBuffer.size() - btOutputOffset);

	if (!output.update(span))
	{
		writeTime.start();
		return;
	}

	writeLatency.start();

	// FIXME: this has the potential to loop forever
	while (true)
	{
		if (bluetoothDevice->setOutputReport())
		{
			break;
		}

		if (bluetoothDevice->nativeError() != ERROR_BUSY)
		{
			closeDeviceAndResetIdle(bluetoothDevice);
			break;
		}

		std::this_thread::sleep_for(1ms);
	}

	writeLatency.stop();
	writeTime.start();
}

void Ds4Device::onDisconnectError(const std::shared_ptr<hid::HidInstance>& device, ConnectionType connectionType)
{
	const size_t nativeError = device->nativeError();

	const auto reason = nativeError != ERROR_DEVICE_NOT_CONNECTED
	                    ? Ds4DisconnectEvent::Reason::error
	                    : Ds4DisconnectEvent::Reason::closed;

	onDisconnect.invoke(this, Ds4DisconnectEvent(connectionType, reason, nativeError));
}

bool Ds4Device::run()
{
	// HACK: make this class manage the light state
	output.lightColor = activeLight.color;

	// HACK: see above
	if (activeLight.idleFade)
	{
		const Ds4LightOptions& l = settings.useProfileLight ? profile.light : settings.light;
		const float m = isIdle() ? 1.0f : std::clamp(static_cast<float>(duration_cast<milliseconds>(idleTime.elapsed()).count())
		                                             / static_cast<float>(duration_cast<milliseconds>(idleTimeout()).count()),
		                                             0.0f, 1.0f);

		output.lightColor = Ds4Color::lerp(l.color, fadeColor, m);
	}

	const bool lastChargingState = charging();
	const uint8_t lastBatteryLevel = battery();

	// cache
	const bool usb = usbConnected();
	const bool bluetooth = bluetoothConnected();

	const ConnectionType preferredConnection = Program::settings.preferredConnection;
	const bool useUsb = usb && (preferredConnection == +ConnectionType::usb || !bluetooth);
	const bool useBluetooth = bluetooth && (preferredConnection == +ConnectionType::bluetooth || !usb);

	bool dataReceived = false;

	auto asyncRead = [this](hid::HidInstance* i) -> bool
	{
		if (!i->isOpen())
		{
			return false;
		}

		if (i->asyncReadPending())
		{
			// WIP
			const bool asyncReadInProgress = i->asyncReadInProgress();
			if (!asyncReadInProgress && i->nativeError() == ERROR_SEM_TIMEOUT)
			{
				Logger::writeLine(LogLevel::warning, this->name(), "Got error 21 on pending read [1]");
			}

			return !asyncReadInProgress;
		}

		if (!i->readAsync())
		{
			// WIP
			if (i->nativeError() == ERROR_SEM_TIMEOUT)
			{
				Logger::writeLine(LogLevel::warning, this->name(), "Got error 21 on active read; ignoring");
			}
			else
			{
				i->close();
			}

			return false;
		}

		const bool asyncReadInProgress = i->asyncReadInProgress(); // WIP

		// WIP
		if (!asyncReadInProgress && i->nativeError() == ERROR_SEM_TIMEOUT)
		{
			Logger::writeLine(LogLevel::warning, this->name(), "Got error 21 on pending read [2]");
		}

		return !asyncReadInProgress;
	};

	if (useUsb)
	{
		writeUsbAsync();

		if (!usbDevice->isOpen())
		{
			onDisconnectError(usbDevice, ConnectionType::usb);
		}
		else if (asyncRead(usbDevice.get()))
		{
			dataReceived = true;

			constexpr auto usbInputOffset = 1;
			const auto span = std::span(&usbDevice->inputBuffer[usbInputOffset],
			                            usbDevice->inputBuffer.size() - usbInputOffset);

			input.update(span);
		}
		else if (!usbDevice->isOpen())
		{
			onDisconnectError(usbDevice, ConnectionType::usb);
		}
	}
	else if (useBluetooth)
	{
		writeBluetooth();

		if (!bluetoothDevice->isOpen())
		{
			onDisconnectError(bluetoothDevice, ConnectionType::bluetooth);
		}
		else if (asyncRead(bluetoothDevice.get()))
		{
			if (bluetoothDevice->inputBuffer[0] == 0x11)
			{
				dataReceived = true;

				constexpr auto btInputOffset = 3;
				const auto span = std::span(&bluetoothDevice->inputBuffer[btInputOffset],
				                            bluetoothDevice->inputBuffer.size() - btInputOffset);

				input.update(span);
			}
		}
		else if (!bluetoothDevice->isOpen())
		{
			onDisconnectError(bluetoothDevice, ConnectionType::bluetooth);
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
				else if (!notifiedCharged && lastChargingState == charging() && battery() >= 10)
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

	return dataReceived;
}

void Ds4Device::controllerThread()
{
	simulator.start();
	readLatency.start();
	idleTime.start();
	writeTime.start();

	while (connected() && running)
	{
		bool dataReceived;

		{
			auto lock_guard = lock();
			dataReceived = run();
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
