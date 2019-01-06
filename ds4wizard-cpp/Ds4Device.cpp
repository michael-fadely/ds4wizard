#include "pch.h"

#include <chrono>
#include <thread>
#include <sstream>
#include <iomanip>

#include "Ds4Device.h"
#include "lock.h"
#include "program.h"
#include "DeviceProfileCache.h"
#include "Bluetooth.h"
#include "Ds4AutoLightColor.h"

using namespace std::chrono;

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
	LOCK(sync);
	return bluetoothDevice != nullptr && bluetoothDevice->isOpen();
}

bool Ds4Device::usbConnected()
{
	LOCK(sync);
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

Ds4Device::Ds4Device(hid::HidInstance& device)
	: simulator(this)
{
	open(device);
}

void Ds4Device::open(hid::HidInstance& device)
{
	std::stringstream macaddr;

	macaddr << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
		<< static_cast<int>(device.serial[0]);

	for (size_t i = 1; i < device.serial.size(); ++i)
	{
		macaddr << ':' << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
			<< static_cast<int>(device.serial[i]);
	}

	macAddress_     = macaddr.str();
	safeMacAddress_ = macAddress_;

	safeMacAddress_.erase(std::remove(safeMacAddress_.begin(), safeMacAddress_.end(), ':'), safeMacAddress_.end());
	std::transform(safeMacAddress_.begin(), safeMacAddress_.end(), safeMacAddress_.begin(), tolower);

	if (device.caps().inputReportSize != 64)
	{
		bluetoothDevice = std::make_unique<hid::HidInstance>(std::move(device));
		setupBluetoothOutputBuffer();
	}
	else
	{
		usbDevice = std::make_unique<hid::HidInstance>(std::move(device));
		setupUsbOutputBuffer();
	}

	auto settings = Program::profileCache.getSettings(macAddress_);
	if (!settings.has_value())
	{
		this->settings = {};
	}
	else
	{
		this->settings = *settings;
	}

	applyProfile();
}

void Ds4Device::saveSettings()
{
	LOCK(sync);
	Program::profileCache.saveSettings(macAddress_, settings);
}

void Ds4Device::applySettings(const DeviceSettings& newSettings)
{
	LOCK(sync);
	settings = newSettings;
	saveSettings();
	applyProfile();
}

void Ds4Device::applyProfile()
{
	LOCK(sync);
	releaseAutoColor();

	auto cachedProfile = Program::profileCache.getProfile(settings.profile);
	if (!cachedProfile.has_value())
	{
		settings.profile = {};
		profile = DeviceProfile::defaultProfile();
	}
	else
	{
		profile = *cachedProfile;
	}

	simulator.applyProfile(&profile);

	Ds4LightOptions& l = settings.useProfileLight ? profile.light : settings.light;

	if (l.automaticColor)
	{
		l.color = Ds4AutoLightColor::getColor(colorIndex);
	}

	activeLight = Ds4LightOptions(l);

	if (usbDevice != nullptr && (!usbConnected() || usbDevice->isExclusive() != profile.exclusiveMode))
	{
		closeUsbDevice();
		hid::HidInstance inst = std::move(*usbDevice);
		openUsbDevice(inst);
	}

	if (bluetoothDevice != nullptr && (!bluetoothConnected() || bluetoothDevice->isExclusive() != profile.exclusiveMode))
	{
		closeBluetoothDevice();
		hid::HidInstance inst = std::move(*bluetoothDevice);
		openBluetoothDevice(inst);
	}

	idleTime.start();
}

void Ds4Device::releaseAutoColor()
{
	LOCK(sync);
	Ds4AutoLightColor::releaseColor(colorIndex);
	colorIndex = -1;
}

void Ds4Device::onProfileChanged(const std::string& newName)
{
	LOCK(sync);
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
	LOCK(sync);
	return readLatency;
}

Latency Ds4Device::getWriteLatency()
{
	LOCK(sync);
	return writeLatency;
}

void Ds4Device::resetReadLatencyPeak()
{
	LOCK(sync);
	readLatency.resetPeak();
}

void Ds4Device::resetWriteLatencyPeak()
{
	LOCK(sync);
	writeLatency.resetPeak();
}

void Ds4Device::closeImpl()
{
	LOCK(sync);
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
	LOCK(sync);

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

	for (size_t i = 0; !Bluetooth::disconnectDevice(bluetoothDevice->serial) && i < 5; i++)
	{
		std::this_thread::sleep_for(125ms);
	}

	closeBluetoothDevice();

	if (reason == BluetoothDisconnectReason::idle)
	{
		onBluetoothIdleDisconnect.invoke(this);
	}
	else
	{
		onBluetoothDisconnected.invoke(this);
	}
}

void Ds4Device::closeUsbDevice()
{
	LOCK(sync);

	if (usbDevice != nullptr && usbDevice->isOpen())
	{
		usbDevice->close();
	}

	idleTime.start();
}

bool Ds4Device::openDevice(hid::HidInstance& device, bool exclusive)
{
	if (device.open((exclusive ? hid::HidOpenFlags::exclusive : 0) | hid::HidOpenFlags::async))
	{
		return true;
	}

	return exclusive && device.open(hid::HidOpenFlags::async);
}

void Ds4Device::openBluetoothDevice(hid::HidInstance& device)
{
	LOCK(sync);
	{
		if (bluetoothConnected())
		{
			return;
		}

		if (!openDevice(device, profile.exclusiveMode))
		{
			// TODO: error handling
			return;
		}

		if (profile.exclusiveMode && !device.isExclusive())
		{
			onBluetoothExclusiveFailure.invoke(this);
		}
		else
		{
			onBluetoothConnected.invoke(this);
		}

		bluetoothDevice = std::make_unique<hid::HidInstance>(std::move(device));

		// Enables bluetooth operational mode which makes
		// the controller send report id 17 (0x11)
		std::array<uint8_t, 37> temp {};
		temp[0] = 0x02;

		if (bluetoothDevice->getFeature(temp))
		{
			// success
		}

		setupBluetoothOutputBuffer();
		idleTime.start();
	}
}

void Ds4Device::openUsbDevice(hid::HidInstance& device)
{
	LOCK(sync);
	{
		if (usbConnected())
		{
			return;
		}

		if (!openDevice(device, profile.exclusiveMode))
		{
			// TODO: error handling
			return;
		}

		if (profile.exclusiveMode && !device.isExclusive())
		{
			onUsbExclusiveFailure.invoke(this);
		}
		else
		{
			onUsbConnected.invoke(this);
		}

		usbDevice = std::make_unique<hid::HidInstance>(std::move(device));
		setupUsbOutputBuffer();
	}
}

void Ds4Device::setupBluetoothOutputBuffer() const
{
	bluetoothDevice->output_buffer[0] = 0x11;
	bluetoothDevice->output_buffer[1] = 0x80; // For HID + CRC, use 0xC0.
	bluetoothDevice->output_buffer[3] = 0x0F;
}

void Ds4Device::setupUsbOutputBuffer() const
{
	usbDevice->output_buffer[0] = 0x05;
	usbDevice->output_buffer[1] = 0xFF;
}

void Ds4Device::writeUsbAsync()
{
	if (usbDevice->checkPendingWrite())
	{
		return;
	}

	writeLatency.stop();
	simulator.readXInput();

	constexpr auto usb_output_offset = 4;

	const auto span = gsl::make_span(&usbDevice->output_buffer[usb_output_offset],
	                                 usbDevice->output_buffer.size() - usb_output_offset);

	if (!output.update(span))
	{
		return;
	}

	usbDevice->writeAsync();
	writeLatency.start();
}

void Ds4Device::writeBluetooth()
{
	simulator.readXInput();

	constexpr auto bt_output_offset = 6;

	const auto span = gsl::make_span(&bluetoothDevice->output_buffer[bt_output_offset],
	                                 bluetoothDevice->output_buffer.size() - bt_output_offset);

	if (!output.update(span))
	{
		return;
	}

	writeLatency.start();

	if (!bluetoothDevice->setOutputReport())
	{
		closeBluetoothDevice();
	}

	writeLatency.stop();
}

void Ds4Device::run()
{
	deltaTime = static_cast<float>(duration_cast<milliseconds>(deltaStopwatch.elapsed()).count());
	deltaStopwatch.start();

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

	const bool charging_   = charging();
	const uint8_t battery_ = battery();

	// cache
	const bool usb = usbConnected();
	const bool bluetooth = bluetoothConnected();

	const ConnectionType preferredConnection = Program::settings.preferredConnection;
	const bool useUsb = usb && (preferredConnection == +ConnectionType::usb || !bluetooth);
	const bool useBluetooth = bluetooth && (preferredConnection == +ConnectionType::bluetooth || !usb);

	dataReceived = false;

	if (useUsb)
	{
		writeUsbAsync();

		if (usbDevice->readAsync())
		{
			dataReceived = true;

			constexpr auto usb_input_offset = 1;
			const auto span = gsl::make_span(&usbDevice->input_buffer[usb_input_offset],
			                                 usbDevice->input_buffer.size() - usb_input_offset);

			input.update(span);
		}

		// If the controller gets disconnected from USB while idle,
		// reset the idle timer so that it doesn't get immediately
		// disconnected from bluetooth (if connected).
		if (!usbConnected())
		{
			idleTime.start();
		}
	}
	else if (useBluetooth)
	{
		writeBluetooth();

		if (bluetoothDevice->readAsync() && bluetoothDevice->input_buffer[0] == 0x11)
		{
			dataReceived = true;

			constexpr auto bt_input_offset = 3;
			const auto span = gsl::make_span(&bluetoothDevice->input_buffer[bt_input_offset],
			                                 bluetoothDevice->input_buffer.size() - bt_input_offset);

			input.update(span);
		}
	}

	const float lx = input.getAxis(Ds4Axis::leftStickX, std::nullopt);
	const float ly = input.getAxis(Ds4Axis::leftStickY, std::nullopt);
	const float ls = std::sqrt(lx * lx + ly * ly);

	const float rx = input.getAxis(Ds4Axis::rightStickX, std::nullopt);
	const float ry = input.getAxis(Ds4Axis::rightStickY, std::nullopt);
	const float rs = std::sqrt(rx * rx + ry * ry);

	// TODO: gyro/accel
	if (input.buttonsChanged || input.heldButtons
	    || ls >= 0.25f || rs >= 0.25f)
	{
		idleTime.start();
	}
	else if (disconnectOnIdle() && useBluetooth && isIdle())
	{
		disconnectBluetooth(BluetoothDisconnectReason::idle);
	}

	if (dataReceived)
	{
		simulator.runMaps();
		readLatency.stop();

		auto average = duration_cast<milliseconds>(readLatency.average());

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

		if (charging_ != charging() || battery_ != battery())
		{
			settings.displayNotifications(this);
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
	readLatency.start();
	idleTime.start();
	deltaStopwatch.start();

	while (connected() && running)
	{
		{
			LOCK(sync);
			run();
		}

		if (!dataReceived)
		{
			std::this_thread::yield();
		}

		std::this_thread::sleep_for(1ms);
	}

	closeImpl();
	onDeviceClosed.invoke(this);
}

void Ds4Device::start()
{
	if (deviceThread == nullptr)
	{
		running = true;
		deviceThread = std::make_unique<std::thread>(&Ds4Device::controllerThread, this);
	}
}
