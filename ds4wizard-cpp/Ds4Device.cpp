#include "stdafx.h"

#include <chrono>
#include <thread>
#include <sstream>
#include <iomanip>

#include <hid_handle.h>
#include <hid_util.h>

#include "Ds4Device.h"
#include "lock.h"
#include "program.h"
#include "DeviceProfileCache.h"
#include "Bluetooth.h"
#include "Ds4AutoLightColor.h"
#include "ScpDevice.h"
#include "Logger.h"

using namespace std::chrono;

// TODO: decouple the logger and use events instead

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

Stopwatch::clock::duration Ds4Device::getLatency()
{
	LOCK(sync);
	return storedLatency;
}

Stopwatch::clock::duration Ds4Device::getLatencyAverage()
{
	LOCK(sync);
	
	if (!latencyPoints)
	{
		return 0ms;
	}

	if (latencyPoints == 1)
	{
		return latencySum;
	}

	return latencySum / latencyPoints;
}

Stopwatch::clock::duration Ds4Device::getLatencyPeak()
{
	LOCK(sync);
	return peakLatency;
}

void Ds4Device::resetLatencyPeak()
{
	LOCK(sync);
	peakLatency = 0ms;
}

uint8_t Ds4Device::battery() const
{
	return input.data.battery;
}

bool Ds4Device::charging() const
{
	return input.data.charging;
}

const std::string& Ds4Device::name() const
{
	return settings.name.empty() ? macAddress : settings.name;
}

Ds4Device::Ds4Device(hid::HidInstance& device)
{
	std::stringstream macaddr;

	macaddr << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
		<< static_cast<int>(device.serial[0]);

	for (size_t i = 1; i < device.serial.size(); ++i)
	{
		macaddr << ':' << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
			<< static_cast<int>(device.serial[i]);
	}

	macAddress = macaddr.str();
	safeMacAddress = macAddress;

	safeMacAddress.erase(std::remove(safeMacAddress.begin(), safeMacAddress.end(), ':'), safeMacAddress.end());
	std::transform(safeMacAddress.begin(), safeMacAddress.end(), safeMacAddress.begin(), tolower);

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

	auto settings = Program::profileCache.getSettings(macAddress);
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
	Program::profileCache.saveSettings(macAddress, settings);
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

	touchRegions.clear();

	for (auto& pair : profile.touchRegions)
	{
		touchRegions.push_back(&pair.second);
	}

	if (profile.useXInput)
	{
		if (!scpDeviceOpen())
		{
			scpDeviceClose();
		}
	}
	else
	{
		scpDeviceClose();
	}

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

bool Ds4Device::scpDeviceOpen()
{
	// TODO: !!! when another handle is opened to the scp device, it fucks everything

	if (scpDevice != nullptr)
	{
		return true;
	}

	// TODO: detect toggle of auto index and disconnect/reconnect the device
	int index = profile.autoXInputIndex ? ScpDevice::getFreePort() : profile.xinputIndex;

	if (index < 0)
	{
		return false;
	}

	std::unique_ptr<hid::HidInstance> info;

	hid::enumerateGuid([&](const std::wstring& path, const std::wstring& instanceId) -> bool
	{
		info = std::make_unique<hid::HidInstance>(path, instanceId, false);
		return true;
	}, GUID_DEVINTERFACE_SCPVBUS);

	if (info == nullptr)
	{
		Logger::writeLine(LogLevel::warning, "ScpVBus device not found. XInput emulation will not be available.");
		return false;
	}

	auto handle = Handle(CreateFile(info->path.c_str(), GENERIC_READ | GENERIC_WRITE,
	                                FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr), true);

	if (!handle.isValid())
	{
		Logger::writeLine(LogLevel::warning, "Failed to acquire ScpVBus device handle. XInput emulation will not be available.");
		return false;
	}

	scpDevice = std::make_unique<ScpDevice>(std::move(handle));

	if (scpDevice->connect(index))
	{
		realXInputIndex = index;
		return true;
	}

	// If connecting an emulated XInput controller failed,
	// it's likely because it's already connected. Disconnect
	// it before continuing.
	scpDevice->disconnect(index);

	bool ok = false;

	// Try up to 4 times to recover the virtual controller.
	for (size_t i = 0; i < 4; i++)
	{
		ok = scpDevice->connect(index);

		if (ok)
		{
			break;
		}

		std::this_thread::yield();
		std::this_thread::sleep_for(250ms);
	}

	if (!ok)
	{
		Logger::writeLine(LogLevel::warning, "Failed to obtain ScpVBus XInput handle. XInput emulation will not be available.");
		return false;
	}

	realXInputIndex = index;
	return true;
}

void Ds4Device::scpDeviceClose()
{
	LOCK(sync);

	if (scpDevice == nullptr)
	{
		return;
	}

	if (realXInputIndex >= 0)
	{
		scpDevice->disconnect(realXInputIndex);
	}

	scpDevice->close();
	scpDevice = nullptr;
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

void Ds4Device::closeImpl()
{
	LOCK(sync);
	running = false;

	closeUsbDevice();
	closeBluetoothDevice();

	scpDeviceClose();
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

void Ds4Device::disconnectBluetooth()
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
			// TODO
			return;
		}

		if (profile.exclusiveMode && !device.isExclusive())
		{
			Logger::writeLine(LogLevel::warning, name(), "Failed to open Bluetooth device exclusively.");
		}
		else
		{
			Logger::writeLine(LogLevel::info, name(), "Bluetooth connected.");
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
			// TODO
			return;
		}

		if (profile.exclusiveMode && !device.isExclusive())
		{
			Logger::writeLine(LogLevel::warning, name(), "Failed to open USB device exclusively.");
		}
		else
		{
			Logger::writeLine(LogLevel::info, name(), "USB connected.");
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

	if (profile.useXInput && scpDevice != nullptr)
	{
		scpDevice->syncState(realXInputIndex);
		output.fromXInput(realXInputIndex, scpDevice);
	}

	constexpr auto usb_output_offset = 4;

	const auto span = gsl::make_span(&usbDevice->output_buffer[usb_output_offset],
	                                 usbDevice->output_buffer.size() - usb_output_offset);

	if (!output.update(span))
	{
		return;
	}

	usbDevice->writeAsync();
}

void Ds4Device::writeBluetooth()
{
	if (profile.useXInput && scpDevice != nullptr)
	{
		scpDevice->syncState(realXInputIndex);
		output.fromXInput(realXInputIndex, scpDevice);
	}

	constexpr auto bt_output_offset = 6;

	const auto span = gsl::make_span(&bluetoothDevice->output_buffer[bt_output_offset],
	                                 bluetoothDevice->output_buffer.size() - bt_output_offset);

	if (!output.update(span))
	{
		return;
	}

	if (!bluetoothDevice->setOutputReport())
	{
		closeBluetoothDevice();
	}
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
		double m = isIdle() ? 1.0 : std::clamp(duration_cast<milliseconds>(idleTime.elapsed()).count()
		                                       / static_cast<double>(duration_cast<milliseconds>(idleTimeout()).count()),
		                                       0.0, 1.0);

		output.lightColor = Ds4Color::lerp(l.color, fadeColor, static_cast<float>(m));
	}

	const bool charging_ = charging();
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
	const auto  ls = static_cast<float>(std::sqrt(lx * lx + ly * ly));

	const float rx = input.getAxis(Ds4Axis::rightStickX, std::nullopt);
	const float ry = input.getAxis(Ds4Axis::rightStickY, std::nullopt);
	const auto  rs = static_cast<float>(std::sqrt(rx * rx + ry * ry));

	// TODO: gyro/accel
	if (input.buttonsChanged || input.heldButtons != 0
	    || ls >= 0.25f || rs >= 0.25f)
	{
		idleTime.start();
	}
	else if (disconnectOnIdle() && useBluetooth && isIdle())
	{
		disconnectBluetooth();
		Logger::writeLine(LogLevel::info, name(), "Bluetooth idle disconnect." /* TODO: std::string.Format(Resources.IdleDisconnect, idleTimeout)*/);
	}

	if (dataReceived)
	{
		runMaps();

		latency.stop();
		storedLatency = latency.elapsed();

		addLatencySum();

		if (latency.elapsed() >= 5ms)
		{
			// TODO: configurable latency target & light flash (although that increases latency on send)
			qDebug() << "latency: " << duration_cast<milliseconds>(latency.elapsed()).count() << " ms";
		}

		latency.start();

		if (profile.useXInput)
		{
			input.toXInput(realXInputIndex, scpDevice);
		}

		if (charging_ != charging() || battery_ != battery())
		{
			settings.displayNotifications(this);
			onBatteryLevelChanged();
		}

#if false
			// Experimental garbage
			if ((Input.PressedButtons & Ds4Buttons.PS) != 0)
			{
				// Set audio output to speaker only (3)
				auto buffer = new byte[3];
				buffer[0] = 0x0E0;
				buffer[2] = 3;

				usbDevice->SetFeature(buffer);

				auto volume = new byte[0x20];
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

				usbDevice->Write(volume);
			}
#endif
	}
	else
	{
		input.updateChangedState();
		runPersistent();

		storedLatency = latency.elapsed();
		addLatencySum();
	}

	if (storedLatency > peakLatency)
	{
		peakLatency = storedLatency;
	}
}

void Ds4Device::controllerThread()
{
	latency.start();
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
	onDeviceClosed();
}

void Ds4Device::onDeviceClosed()
{
	deviceClosed.invoke(this, nullptr);
}

void Ds4Device::addLatencySum()
{
	if (!latencyPoints)
	{
		latencySum = latency.elapsed();
	}
	else
	{
		latencySum += latency.elapsed();
	}

	++latencyPoints;

	if (latencySum >= 1s)
	{
		latencySum /= latencyPoints;
		latencyPoints = 1;
	}
}

void Ds4Device::start()
{
	if (deviceThread == nullptr)
	{
		running = true;
		deviceThread = std::make_unique<std::thread>(&Ds4Device::controllerThread, this);
	}
}

void Ds4Device::onBatteryLevelChanged()
{
	batteryLevelChanged.invoke(this, nullptr);
}

void Ds4Device::simulateXInputButton(XInputButtons_t buttons, PressedState state)
{
	XInputButtons_t dest = input.gamepad.wButtons;

	switch (state)
	{
		case PressedState::off:
			break;

		case PressedState::released:
			dest &= ~buttons;
			break;

		case PressedState::on:
		case PressedState::pressed:
			dest |= buttons;
			break;

		default:
			throw /* TODO new ArgumentOutOfRangeException(nameof(state), state, "Invalied pressed state.")*/;
	}

	input.gamepad.wButtons = dest;
}

void Ds4Device::simulateXInputAxis(XInputAxes& axes, float m)
{
	for (XInputAxis_t bit : XInputAxis_values)
	{
		if (!(axes.axes & bit))
		{
			continue;
		}

		AxisOptions options = axes.getAxisOptions(static_cast<XInputAxis::T>(bit));

		const auto trigger = static_cast<uint8_t>(255.0f * m);

		const auto axis = static_cast<short>(std::numeric_limits<short>::max() * m);

		const short workAxis = options.polarity == +AxisPolarity::negative
			                       ? static_cast<short>(-axis)
			                       : axis;

		const bool isFirst = !(simulatedXInputAxis & bit);
		simulatedXInputAxis |= bit;

		switch (bit)
		{
			case XInputAxis::leftStickX:
				if (isFirst || axis > std::abs(input.gamepad.sThumbLX))
				{
					input.gamepad.sThumbLX = workAxis;
				}

				break;

			case XInputAxis::leftStickY:
				if (isFirst || axis > std::abs(input.gamepad.sThumbLY))
				{
					input.gamepad.sThumbLY = workAxis;
				}

				break;

			case XInputAxis::rightStickX:
				if (isFirst || axis > std::abs(input.gamepad.sThumbRX))
				{
					input.gamepad.sThumbRX = workAxis;
				}

				break;

			case XInputAxis::rightStickY:
				if (isFirst || axis > std::abs(input.gamepad.sThumbRY))
				{
					input.gamepad.sThumbRY = workAxis;
				}

				break;

			case XInputAxis::leftTrigger:
				if (isFirst || trigger > input.gamepad.bLeftTrigger)
				{
					input.gamepad.bLeftTrigger = trigger;
				}

				break;

			case XInputAxis::rightTrigger:
				if (isFirst || trigger > input.gamepad.bRightTrigger)
				{
					input.gamepad.bRightTrigger = trigger;
				}

				break;

			default:
				throw /* TODO: new ArgumentOutOfRangeException(nameof(options), options, "Invalid XInput axis.")*/;
		}
	}
}

// TODO: OPTIMIZE !!!
void Ds4Device::getActive(InputMapBase& map)
{
	if (!__maps.empty())
	{
		return;
	}

	for (InputModifier& mod : profile.modifiers)
	{
		if (!mod.isActive())
		{
			continue;
		}

		for (InputMap& binding : mod.bindings)
		{
			if (&binding != &map)
			{
				__maps.push_back(&binding);
			}
		}
	}
}

bool Ds4Device::isOverriddenByModifierSet(InputMapBase& map)
{
	if (profile.modifiers.empty())
	{
		return false;
	}

	__maps.clear();

	/*std::deque<InputMap> maps = Profile.Modifiers
		.Where(x = > x.IsActive)
		.Select(y = > y.Bindings)
		.Where(x = > x != nullptr)
		.SelectMany(x = > x)
		.Where(x = > !ReferenceEquals(x, map))
		.ToList();*/

	if ((map.inputType & InputType::button) != 0)
	{
		getActive(map);

		if (std::any_of(__maps.begin(), __maps.end(), [&](InputMap* x) -> bool
		{
			return (x->inputType & InputType::button) != 0 && (x->inputButtons.value_or(0) & map.inputButtons.value_or(0)) != 0;
		}))
		{
			return true;
		}
	}

	if ((map.inputType & InputType::axis) != 0)
	{
		getActive(map);

		if (std::any_of(__maps.begin(), __maps.end(), [&](InputMap* x) -> bool
		{
			return (x->inputType & InputType::axis) != 0 && (x->inputAxis.value_or(0) & map.inputAxis.value_or(0)) != 0;
		}))
		{
			return true;
		}
	}

	if ((map.inputType & InputType::touchRegion) != 0)
	{
		getActive(map);

		if (std::any_of(__maps.begin(), __maps.end(), [&](InputMap* x) -> bool
		{
			return (x->inputType & InputType::touchRegion) != 0 && x->inputRegion == map.inputRegion;
		}))
		{
			return true;
		}
	}

	return false;
}

void Ds4Device::runMap(InputMap& m, InputModifier* modifier)
{
	if (m.inputType == 0)
	{
		throw /* TODO: new ArgumentOutOfRangeException(nameof(m.inputType), m.inputType, "InputType must be non-zero.")*/;
	}

	for (InputType_t value : InputType_values)
	{
		if ((m.inputType & value) == 0)
		{
			continue;
		}

		switch (value)
		{
			case InputType::button:
			{
				PressedState state = m.simulatedState();
				applyMap(m, modifier, state, m.isActive() && ((modifier && modifier->isActive()) || m.isToggled) ? 1.0f : 0.0f);
				break;
			}

			case InputType::axis:
			{
				if (m.inputAxis.value_or(0) == 0)
				{
					throw /* TODO: new ArgumentNullException(nameof(m.InputAxis))*/;
				}

				for (Ds4Axis_t bit : Ds4Axis_values)
				{
					if (!(m.inputAxis.value() & bit))
					{
						continue;
					}

					InputAxisOptions options = m.getAxisOptions(bit);

					float analog = input.getAxis(m.inputAxis.value(), options.polarity);
					options.applyDeadZone(analog);

					PressedState state = m.simulatedState();
					applyMap(m, modifier, state, analog);
				}

				break;
			}

			case InputType::touchRegion:
			{
				Ds4TouchRegion& region = profile.touchRegions[m.inputRegion];

				if (region.type == +Ds4TouchRegionType::button || !m.touchDirection.has_value() || m.touchDirection == Direction::none)
				{
					PressedState state = handleTouchToggle(m, modifier, region.state1);
					applyMap(m, modifier, state, Pressable::isActiveState(state) ? 1.0f : 0.0f);

					state = handleTouchToggle(m, modifier, region.state2);
					applyMap(m, modifier, state, Pressable::isActiveState(state) ? 1.0f : 0.0f);
				}
				else
				{
					Direction_t direction = m.touchDirection.value();

					float deadZone = region.getDeadZone(direction);

					PressedState state = handleTouchToggle(m, modifier, region.state1);
					float analog = region.getTouchDelta(Ds4Buttons::touch1, direction, input.data.touchPoint1);

					if (analog < deadZone)
					{
						Pressable::release(state);
					}

					region.applyDeadZone(direction, analog);
					applyMap(m, modifier, state, analog);

					state  = handleTouchToggle(m, modifier, region.state2);
					analog = region.getTouchDelta(Ds4Buttons::touch2, direction, input.data.touchPoint2);

					if (analog < deadZone)
					{
						Pressable::release(state);
					}

					region.applyDeadZone(direction, analog);
					applyMap(m, modifier, state, analog);
				}

				return;
			}

			default:
				throw /* TODO: new ArgumentOutOfRangeException()*/;
		}
	}
}

PressedState Ds4Device::handleTouchToggle(InputMap& m, InputModifier* modifier, const Pressable& pressable)
{
	if (m.touchDirection.has_value() && m.touchDirection != Direction::none)
	{
		return m.isToggled ? m.simulatedState() : pressable.pressedState;
	}

	if (m.rapidFire == true)
	{
		return m.simulatedState();
	}

	PressedState state = (m.isToggled || (modifier && modifier->isActive())) ? pressable.pressedState : m.pressedState;

	if (!Pressable::isActiveState(state))
	{
		state = m.simulatedState();
	}

	return state;
}

void Ds4Device::applyMap(InputMap& m, InputModifier* modifier, PressedState state, float analog)
{
#if true
	switch (m.simulatorType)
	{
		case SimulatorType::input:
			switch (m.outputType)
			{
				case OutputType::xinput:
					if (m.xinputButtons.has_value())
					{
						simulateXInputButton(m.xinputButtons.value(), state);
					}

					if (m.xinputAxes.has_value())
					{
						simulateXInputAxis(m.xinputAxes.value(), analog);
					}

					break;

				case OutputType::keyboard:
					simulateKeyboard(m, state);
					break;

					// TODO: AxisOptions thing for mouse
				case OutputType::mouse:
					simulateMouse(m, state, analog);
					break;

				default:
					throw /* TODO: new ArgumentOutOfRangeException(nameof(m.outputType), m.outputType, "Invalid output type.")*/;
			}

			break;

		case SimulatorType::action:
			if (!m.action.has_value() || m.action.value() == +ActionType::none)
			{
				throw /* TODO: new ArgumentNullException(nameof(m.action))*/;
			}

			if (m.isActive() && (modifier && modifier->isActive()))
			{
				runAction(m.action.value());
			}

			break;

		default:
			throw /* TODO: new ArgumentOutOfRangeException(nameof(m.simulatorType), m.simulatorType, "Invalid map type.")*/;
	}
#endif
}

void Ds4Device::simulateMouse(const InputMap& m, PressedState state, float analog)
{
	// TODO
#if 0
	if (m.MouseButton.HasValue)
	{
		switch (state)
		{
			case PressedState::Pressed:
				switch (m.MouseButton.Value)
				{
					case MouseButton.LeftButton:
						MouseSimulator.LeftButtonDown();
						break;
					case MouseButton.MiddleButton:
						MouseSimulator.MiddleButtonDown();
						break;
					case MouseButton.RightButton:
						MouseSimulator.RightButtonDown();
						break;
				}

				break;

			case PressedState::Released:
				switch (m.MouseButton.Value)
				{
					case MouseButton.LeftButton:
						MouseSimulator.LeftButtonUp();
						break;
					case MouseButton.MiddleButton:
						MouseSimulator.MiddleButtonUp();
						break;
					case MouseButton.RightButton:
						MouseSimulator.RightButtonUp();
						break;
				}

				break;
		}
	}

	if (m.MouseAxes == nullptr)
	{
		return;
	}

	analog *= deltaTime;

	// TODO: /!\ actually the thing (GetAxisOptions)
	Direction direction = m.MouseAxes.Directions;

	int x = 0, y = 0;

	if ((direction & (Direction::Left | Direction::Right)) != (Direction::Left | Direction::Right))
	{
		if ((direction & Direction::Right) != 0)
		{
			x = (int)analog;
		}
		else if ((direction & Direction::Left) != 0)
		{
			x = (int)-analog;
		}
	}
	else
	{
		x = 0;
	}

	if ((direction & (Direction::Up | Direction::Down)) != (Direction::Up | Direction::Down))
	{
		if ((direction & Direction::Up) != 0)
		{
			y = (int)-analog;
		}
		else if ((direction & Direction::Down) != 0)
		{
			y = (int)analog;
		}
	}
	else
	{
		y = 0;
	}

	if (x != 0 || y != 0)
	{
		MouseSimulator.MoveMouseBy(x, y);
	}
#endif
}

void Ds4Device::simulateKeyboard(const InputMap& m, PressedState state)
{
	// TODO
#if 0
	if (m.KeyCode == nullptr)
	{
		return;
	}

	VirtualKeyCode keyCode = m.KeyCode.Value;

	switch (state)
	{
		case PressedState::Pressed:
			KeyboardSimulator.KeyDown(keyCode);

			if (m.KeyCodeModifiers != nullptr)
			{
				foreach(VirtualKeyCode k in m.KeyCodeModifiers)
				{
					KeyboardSimulator.KeyDown(k);
				}
			}

			break;

		case PressedState::Released:
			KeyboardSimulator.KeyUp(keyCode);

			if (m.KeyCodeModifiers != nullptr)
			{
				foreach(VirtualKeyCode k in m.KeyCodeModifiers)
				{
					KeyboardSimulator.KeyUp(k);
				}
			}

			break;
	}
#endif
}

void Ds4Device::runAction(ActionType action)
{
	switch (action)
	{
		case ActionType::bluetoothDisconnect:
			if (bluetoothConnected())
			{
				disconnectBluetooth();
				Logger::writeLine(LogLevel::info, name(), "Bluetooth disconnected.");
			}

			break;

		default:
			throw /* TODO: new ArgumentOutOfRangeException(nameof(action), action, "Invalid action type.")*/;
	}
}

void Ds4Device::runMaps()
{
	simulatedXInputAxis = 0;

	for (InputModifier& modifier : profile.modifiers)
	{
		updatePressedState(modifier);
	}

	if (input.touchChanged)
	{
		updateTouchRegions();
	}

	for (InputMap& m : profile.bindings)
	{
		updateBindingState(m, nullptr);
	}
}

void Ds4Device::runPersistent()
{
	simulatedXInputAxis = 0;

	for (InputModifier& modifier : profile.modifiers)
	{
		if (modifier.isPersistent())
		{
			updatePressedState(modifier);
		}
	}

	for (InputMap& m : profile.bindings)
	{
		if (m.isPersistent())
		{
			updateBindingState(m, nullptr);
		}
	}
}

void Ds4Device::updateTouchRegions()
{
	Ds4Buttons_t disallow = 0;

	//foreach(Ds4TouchRegion region in Profile.TouchRegions.Values.OrderBy(x = > !x.IsActive(touchMask) && !x.AllowCrossOver))

	std::sort(touchRegions.begin(), touchRegions.end(), [](const Ds4TouchRegion* a, const Ds4TouchRegion* b)
	{
		return (a->isActive(touchMask) && !a->allowCrossOver) && !(b->isActive(touchMask) && !b->allowCrossOver);
	});

	for (auto& region : touchRegions)
	{
		if ((input.heldButtons & touchMask) == 0)
		{
			region->setInactive(touchMask);
			continue;
		}

		updateTouchRegion(*region, /* TODO */ nullptr, Ds4Buttons::touch1, input.data.touchPoint1, disallow);
		updateTouchRegion(*region, /* TODO */ nullptr, Ds4Buttons::touch2, input.data.touchPoint2, disallow);
	}
}

void Ds4Device::updateTouchRegion(Ds4TouchRegion& region, InputModifier* modifier, Ds4Buttons_t sender, Ds4Vector2& point, Ds4Buttons_t& disallow)
{
	if ((disallow & sender) == 0 && (input.heldButtons & sender) != 0)
	{
		if (region.isInRegion(sender, point))
		{
			if (modifier && !modifier->isActive())
			{
				updatePressedState(*modifier);
			}

			if (!modifier || modifier->isActive())
			{
				region.setActive(sender, point);

				if (!region.allowCrossOver)
				{
					disallow |= sender;
				}

				return;
			}
		}
	}

	region.setInactive(sender);
}

void Ds4Device::updatePressedStateImpl(InputMapBase& instance, const std::function<void()>& press, const std::function<void()>& release)
{
	if (isOverriddenByModifierSet(instance))
	{
		release();
		return;
	}

	for (InputType_t value : InputType_values)
	{
		if ((instance.inputType & value) == 0)
		{
			continue;
		}

		switch (value)
		{
			case InputType::button:
				if ((instance.inputButtons.value() & input.heldButtons) == instance.inputButtons)
				{
					press();
				}
				else
				{
					release();
				}

				break;

			case InputType::axis:
			{
				if (!instance.inputAxis.has_value() || instance.inputAxis.value() == 0)
				{
					throw /* TODO: new ArgumentNullException(nameof(instance.InputAxis))*/;
				}

				const gsl::span<const Ds4Axis_t> s(Ds4Axis_values);

				const size_t target = std::count_if(s.begin(), s.end(), [&](Ds4Axis_t x) -> bool { return (x & instance.inputAxis.value_or(0)) != 0; });
				size_t count = 0;

				for (Ds4Axis_t bit : Ds4Axis_values)
				{
					if (!(instance.inputAxis.value() & bit))
					{
						continue;
					}

					InputAxisOptions options = instance.getAxisOptions(bit);

					// ReSharper disable once PossibleInvalidOperationException
					float axis = input.getAxis(instance.inputAxis.value(), options.polarity);

					if (axis >= options.deadZone.value_or(0.0f))
					{
						++count;
					}
					else
					{
						break;
					}
				}

				if (count == target)
				{
					press();
				}
				else
				{
					release();
				}

				break;
			}

			case InputType::touchRegion:
			{
				auto it = profile.touchRegions.find(instance.inputRegion);
				if (it == profile.touchRegions.end())
				{
					break;
				}

				auto& region = it->second;

				if (region.isActive(touchMask))
				{
					press();
				}
				else
				{
					release();
				}

				break;
			}

			case InputType::none:
				break;

			default:
				throw; // TODO
		}
	}
}

void Ds4Device::updatePressedState(InputModifier& modifier)
{
	const auto a = [&]() -> void
	{
		modifier.press();
	};

	const auto b = [&]() -> void
	{
		modifier.release();
	};

	updatePressedStateImpl(modifier, a, b);

	if (modifier.bindings.empty())
	{
		return;
	}

	for (InputMap& bind : modifier.bindings)
	{
		updateBindingState(bind, &modifier);
	}
}

void Ds4Device::updatePressedState(InputMap& map, InputModifier* modifier)
{
	const auto a = [&]() -> void
	{
		map.pressModifier(modifier);
	};

	const auto b = [&]() -> void
	{
		map.release();
	};

	updatePressedStateImpl(map, a, b);
}

void Ds4Device::updateBindingState(InputMap& m, InputModifier* modifier)
{
	if (modifier != nullptr)
	{
		if (m.toggle != true)
		{
			if (!modifier->isActive())
			{
				m.release();
				runMap(m, modifier);
				return;
			}
		}
	}

	switch (m.inputType)
	{
		case InputType::touchRegion:
		{
			auto it = profile.touchRegions.find(m.inputRegion);
			if (it == profile.touchRegions.end())
			{
				break;
			}

			auto& region = it->second;

			if (m.touchDirection == Direction::none)
			{
				if (region.isActive(touchMask))
				{
					m.pressModifier(modifier);
				}
				else
				{
					m.release();
				}
			}

			break;
		}

		default:
			updatePressedState(m, modifier);
			break;
	}

	runMap(m, modifier);
}
