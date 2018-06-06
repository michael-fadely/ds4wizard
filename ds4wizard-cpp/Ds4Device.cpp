#include "stdafx.h"

#include <chrono>
#include <thread>
#include <sstream>
#include <iomanip>

#include "Ds4Device.h"
#include "lock.h"
#include "program.h"
#include "DeviceProfileCache.h"
#include "util.h"
#include "Bluetooth.h"
#include "Ds4AutoLightColor.h"
#include "ScpDevice.h"
#include <locale>

using namespace std::chrono;

bool Ds4Device::DisconnectOnIdle() const
{
	return Settings.UseProfileIdle ? Profile.Idle.Disconnect : Settings.Idle.Disconnect;
}

nanoseconds Ds4Device::IdleTimeout() const
{
	return Settings.UseProfileIdle ? Profile.Idle.Timeout : Settings.Idle.Timeout;
}

bool Ds4Device::IsIdle() const
{
	return idleTime.elapsed() >= IdleTimeout();
}

bool Ds4Device::BluetoothConnected()
{
	lock(sync);
	return bluetoothDevice != nullptr && bluetoothDevice->is_open();
}

bool Ds4Device::UsbConnected()
{
	lock(sync);
	return usbDevice != nullptr && usbDevice->is_open();
}

bool Ds4Device::Connected()
{
	return BluetoothConnected() || UsbConnected();
}

uint8_t Ds4Device::Battery() const
{
	return Input.Data.Battery;
}

bool Ds4Device::Charging() const
{
	return Input.Data.Charging;
}

const std::string& Ds4Device::Name() const
{
	return Settings.Name.empty() ? MacAddress : Settings.Name;
}

Ds4Device::Ds4Device(hid::HidInstance& device)
{
	std::stringstream macAddress;

	macAddress << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
		<< static_cast<int>(device.serial[0]);

	for (size_t i = 1; i < device.serial.size(); ++i)
	{
		macAddress << ':' << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
			<< static_cast<int>(device.serial[i]);
	}

	MacAddress = macAddress.str();
	SafeMacAddress = MacAddress;

	SafeMacAddress.erase(std::remove(SafeMacAddress.begin(), SafeMacAddress.end(), ':'), SafeMacAddress.end());
	std::transform(SafeMacAddress.begin(), SafeMacAddress.end(), SafeMacAddress.begin(), tolower);

	if (device.caps().input_report_size != 64)
	{
		bluetoothDevice = std::make_unique<hid::HidInstance>(std::move(device));
		SetupBluetoothOutputBuffer();
	}
	else
	{
		usbDevice = std::make_unique<hid::HidInstance>(std::move(device));
		SetupUsbOutputBuffer();
	}

	auto settings = Program::ProfileCache.GetSettings(MacAddress);
	if (settings == nullptr)
	{
		Settings = {};
	}
	else
	{
		Settings = *settings;
	}

	ApplyProfile();
}

void Ds4Device::SaveSettings()
{
	lock(sync);
	Program::ProfileCache.SaveSettings(MacAddress, Settings);
}

void Ds4Device::ApplyProfile()
{
	lock(sync);
	ReleaseAutoColor();

	auto profile = Program::ProfileCache.GetProfile(Settings.Profile);
	if (profile == nullptr)
	{
		Settings.Profile = {};
		Profile = DeviceProfile::Default();
	}
	else
	{
		Profile = *profile;
	}

	touchRegions.clear();

	for (auto& pair : Profile.TouchRegions)
	{
		touchRegions.push_back(&pair.second);
	}

	if (Profile.UseXInput)
	{
		if (!ScpDeviceOpen())
		{
			ScpDeviceClose();
		}
	}
	else
	{
		ScpDeviceClose();
	}

	Ds4LightOptions l = Settings.UseProfileLight ? Profile.Light : Settings.Light;

	if (l.AutomaticColor)
	{
		l.Color = Ds4AutoLightColor::GetColor(colorIndex);
	}

	activeLight = Ds4LightOptions(l);

	if (usbDevice != nullptr && (!UsbConnected() || usbDevice->is_exclusive() != Profile.ExclusiveMode))
	{
		CloseUsbDevice();
		hid::HidInstance inst = std::move(*usbDevice);
		OpenUsbDevice(inst);
	}

	if (bluetoothDevice != nullptr && (!BluetoothConnected() || bluetoothDevice->is_exclusive() != Profile.ExclusiveMode))
	{
		CloseBluetoothDevice();
		hid::HidInstance inst = std::move(*bluetoothDevice);
		OpenBluetoothDevice(inst);
	}

	idleTime.start();
}

bool Ds4Device::ScpDeviceOpen()
{
	if (scpDevice != nullptr)
	{
		return true;
	}

	// TODO: detect toggle of auto index and disconnect/reconnect the device
	int index = Profile.AutoXInputIndex ? ScpDevice::GetFreePort() : Profile.XInputIndex;

	if (index < 0)
	{
		return false;
	}

	std::unique_ptr<hid::HidInstance> info;

	hid::enum_guid([&](const std::wstring& path, const std::wstring& instanceId) -> bool
	{
		info = std::make_unique<hid::HidInstance>(path, instanceId, true);
		return true;
	}, GUID_DEVINTERFACE_SCPVBUS);

	if (info == nullptr)
	{
		// TODO: Logger.WriteLine(LogLevel.Warning, Resources.ScpVBusMissing);
		return false;
	}

	HANDLE handle = CreateFile(info->path.c_str(), GENERIC_READ | GENERIC_WRITE,
	                           FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);

	if (handle == INVALID_HANDLE_VALUE || handle == nullptr)
	{
		// TODO: Logger.WriteLine(LogLevel.Warning, Resources.ScpVBusOpenFailed);
		return false;
	}

	scpDevice = std::make_unique<ScpDevice>(handle);

	if (scpDevice->Connect(index))
	{
		realXInputIndex = index;
		return true;
	}

	// If connecting an emulated XInput controller failed,
	// it's likely because it's already connected. Disconnect
	// it before continuing.
	scpDevice->Disconnect(index);

	bool ok = false;

	// Try up to 4 times to recover the virtual controller.
	for (size_t i = 0; i < 4; i++)
	{
		ok = scpDevice->Connect(index);

		if (ok)
		{
			break;
		}

		std::this_thread::yield();
		std::this_thread::sleep_for(250ms);
	}

	if (!ok)
	{
		// TODO: Logger.WriteLine(LogLevel.Warning, Resources.XInputCreateFailed);
		return false;
	}

	realXInputIndex = index;
	return true;
}

void Ds4Device::ScpDeviceClose()
{
	lock(sync);

	if (scpDevice == nullptr)
	{
		return;
	}

	if (realXInputIndex >= 0)
	{
		scpDevice->Disconnect(realXInputIndex);
	}

	scpDevice->Close();
	scpDevice = nullptr;
}

void Ds4Device::ReleaseAutoColor()
{
	lock(sync);
	Ds4AutoLightColor::ReleaseColor(colorIndex);
	colorIndex = -1;
}

void Ds4Device::OnProfileChanged(const std::string& newName)
{
	lock(sync);
	Settings.Profile = newName.empty() ? std::string() : newName;
	SaveSettings();
	ApplyProfile();
}

Ds4Device::~Ds4Device()
{
	Close();
}

void Ds4Device::closeImpl()
{
	if (!Connected())
	{
		return;
	}

	{
		lock(sync);

		CloseUsbDevice();
		CloseBluetoothDevice();
		ScpDeviceClose();
		ReleaseAutoColor();
	}

	OnDeviceClosed();
}

void Ds4Device::Close()
{
	if (!deviceThread)
	{
		closeImpl();
		return;
	}

	running = false;
	deviceThread->join();
	deviceThread = nullptr;
}

void Ds4Device::CloseBluetoothDevice()
{
	lock(sync);

	if (bluetoothDevice != nullptr && bluetoothDevice->is_open())
	{
		bluetoothDevice->close();
	}

	idleTime.start();
}

void Ds4Device::DisconnectBluetooth()
{
	if (!BluetoothConnected())
	{
		return;
	}

	for (size_t i = 0; !Bluetooth::DisconnectDevice(bluetoothDevice->serial) && i < 5; i++)
	{
		std::this_thread::sleep_for(125ms);
	}

	CloseBluetoothDevice();
}

void Ds4Device::CloseUsbDevice()
{
	lock(sync);

	if (usbDevice != nullptr && usbDevice->is_open())
	{
		usbDevice->close();
	}

	idleTime.start();
}

bool Ds4Device::OpenDevice(hid::HidInstance& device, bool exclusive)
{
	if (device.open((exclusive ? hid::HidOpenFlags::exclusive : 0) | hid::HidOpenFlags::async))
	{
		return true;
	}

	return exclusive && device.open(hid::HidOpenFlags::async);
}

void Ds4Device::OpenBluetoothDevice(hid::HidInstance& device)
{
	lock(sync);
	{
		if (BluetoothConnected())
		{
			return;
		}

		if (!OpenDevice(device, Profile.ExclusiveMode))
		{
			// TODO
			return;
		}

		if (Profile.ExclusiveMode && !device.is_exclusive())
		{
			// TODO: Logger.WriteLine(LogLevel.Warning, Name, Resources.BluetoothExclusiveOpenFailed);
		}
		else
		{
			// TODO: Logger.WriteLine(LogLevel.Info, Name, Resources.BluetoothConnected);
		}

		bluetoothDevice = std::make_unique<hid::HidInstance>(std::move(device));

		// Enables Bluetooth operational mode which makes
		// the controller send report id 17 (0x11)
		std::array<uint8_t, 37> temp {};
		temp[0] = 0x02;

		if (bluetoothDevice->get_feature(temp))
		{
			// success
		}

		SetupBluetoothOutputBuffer();
		idleTime.start();
	}
}

void Ds4Device::OpenUsbDevice(hid::HidInstance& device)
{
	lock(sync);
	{
		if (UsbConnected())
		{
			return;
		}

		if (!OpenDevice(device, Profile.ExclusiveMode))
		{
			// TODO
			return;
		}

		if (Profile.ExclusiveMode && !device.is_exclusive())
		{
			// TODO: Logger.WriteLine(LogLevel.Warning, Name, Resources.UsbExclusiveOpenFailed);
		}
		else
		{
			// TODO: Logger.WriteLine(LogLevel.Info, Name, Resources.UsbConnected);
		}

		usbDevice = std::make_unique<hid::HidInstance>(std::move(device));
		SetupUsbOutputBuffer();
	}
}

void Ds4Device::SetupBluetoothOutputBuffer() const
{
	bluetoothDevice->output_buffer[0] = 0x11;
	bluetoothDevice->output_buffer[1] = 0x80; // For HID + CRC, use 0xC0.
	bluetoothDevice->output_buffer[3] = 0x0F;
}

void Ds4Device::SetupUsbOutputBuffer() const
{
	usbDevice->output_buffer[0] = 0x05;
	usbDevice->output_buffer[1] = 0xFF;
}

void Ds4Device::WriteUsbAsync()
{
	if (usbDevice->pending_write())
	{
		return;
	}

	if (Profile.UseXInput && scpDevice != nullptr)
	{
		scpDevice->SyncState(realXInputIndex);
		Output.FromXInput(realXInputIndex, scpDevice);
	}

	if (!Output.Update(usbDevice->output_buffer, 4))
	{
		return;
	}

	usbDevice->writeAsync();
}

void Ds4Device::WriteBluetooth()
{
	if (Profile.UseXInput && scpDevice != nullptr)
	{
		scpDevice->SyncState(realXInputIndex);
		Output.FromXInput(realXInputIndex, scpDevice);
	}

	if (!Output.Update(bluetoothDevice->output_buffer, 6))
	{
		return;
	}

	if (!bluetoothDevice->set_output_report())
	{
		CloseBluetoothDevice();
	}
}

void Ds4Device::Run()
{
	deltaTime = static_cast<float>(duration_cast<milliseconds>(deltaStopwatch.elapsed()).count());
	deltaStopwatch.start();

	// HACK: make this class manage the light state
	Output.LightColor = activeLight.Color;

	// HACK: see above
	/*if (activeLight.IdleFade)
	{
		Ds4LightOptions l = Settings.UseProfileLight ? Profile.Light : Settings.Light;
		double m = IsIdle() ? 1.0 : std::clamp(duration_cast<milliseconds>(idleTime.elapsed()).count() / static_cast<double>(duration_cast<milliseconds>(IdleTimeout()).count()), 0.0, 1.0);

		Output.LightColor = Ds4Color::Lerp(l.Color, fadeColor, static_cast<float>(m));
	}*/

	const bool charging = Charging();
	const uint8_t battery = Battery();

	// cache
	const bool usb = UsbConnected();
	const bool bluetooth = BluetoothConnected();

	const ConnectionType preferredConnection = Program::settings.preferredConnection;
	const bool useUsb = usb && (preferredConnection == +ConnectionType::usb || !bluetooth);
	const bool useBluetooth = bluetooth && (preferredConnection == +ConnectionType::bluetooth|| !usb);

	DataReceived = false;

	if (useUsb)
	{
		WriteUsbAsync();

		if (usbDevice->readAsync())
		{
			DataReceived = true;
			Input.Update(usbDevice->input_buffer, 1);
		}

		// If the controller gets disconnected from USB while idle,
		// reset the idle timer so that it doesn't get immediately
		// disconnected from bluetooth (if connected).
		if (!UsbConnected())
		{
			idleTime.start();
		}
	}
	else if (useBluetooth)
	{
		WriteBluetooth();

		if (bluetoothDevice->readAsync() && bluetoothDevice->input_buffer[0] == 0x11)
		{
			DataReceived = true;
			Input.Update(bluetoothDevice->input_buffer, 3);
		}
	}

	const float lx = Input.GetAxis(Ds4Axis::leftStickX, nullptr);
	const float ly = Input.GetAxis(Ds4Axis::leftStickY, nullptr);
	const auto  ls = static_cast<float>(std::sqrt(lx * lx + ly * ly));

	const float rx = Input.GetAxis(Ds4Axis::rightStickX, nullptr);
	const float ry = Input.GetAxis(Ds4Axis::rightStickY, nullptr);
	const auto  rs = static_cast<float>(std::sqrt(rx * rx + ry * ry));

	// TODO: gyro/accel
	if (Input.ButtonsChanged || Input.HeldButtons != 0
	    || ls >= 0.25f || rs >= 0.25f)
	{
		idleTime.start();
	}
	else if (DisconnectOnIdle() && useBluetooth && IsIdle())
	{
		DisconnectBluetooth();
		// TODO: Logger.WriteLine(LogLevel.Info, Name, std::string.Format(Resources.IdleDisconnect, IdleTimeout));
	}

	if (DataReceived)
	{
		 RunMaps();

		if (Latency.elapsed() >= 5ms)
		{
			// TODO: configurable latency target & light flash (although that increases latency on send)
			// TODO: Debug.WriteLine($"{Latency.Elapsed.TotalMilliseconds} ms");
		}

		Latency.start();

		if (Profile.UseXInput)
		{
			Input.ToXInput(realXInputIndex, scpDevice);
		}

		if (charging != Charging() || battery != Battery())
		{
			Settings.DisplayNotifications(this);
			OnBatteryLevelChanged();
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

		std::this_thread::yield();
	}
	else
	{
		Input.UpdateChangedState();
		RunPersistent();
		std::this_thread::sleep_for(1ms);
	}
}

void Ds4Device::ControllerThread()
{
	Latency.start();
	idleTime.start();
	deltaStopwatch.start();

	while (Connected() && running)
	{
		lock(sync);
		Run();
	}

	closeImpl();
}

void Ds4Device::OnDeviceClosed()
{
	// TODO: DeviceClosed.Invoke(this, EventArgs.Empty);
}

void Ds4Device::Start()
{
	if (deviceThread == nullptr)
	{
		running = true;
		deviceThread = std::make_unique<std::thread>(&Ds4Device::ControllerThread, this);
	}
}

void Ds4Device::OnBatteryLevelChanged()
{
	// TODO: BatteryLevelChanged.Invoke(this, EventArgs.Empty);
}

void Ds4Device::SimulateXInputButton(XInputButtons_t buttons, PressedState state)
{
	XInputButtons_t dest = Input.Gamepad.wButtons;

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

	Input.Gamepad.wButtons = dest;
}

void Ds4Device::SimulateXInputAxis(XInputAxes& axes, float m)
{
	for (XInputAxis_t bit : XInputAxis_values)
	{
		if ((axes.Axes & bit) == 0)
		{
			continue;
		}

		AxisOptions options = axes.GetAxisOptions(static_cast<XInputAxis::T>(bit));

		auto trigger = static_cast<uint8_t>(255.0f * m);

		auto axis = static_cast<short>(std::numeric_limits<short>::max() * m);
		short workAxis = options.Polarity == +AxisPolarity::negative ? static_cast<short>(-axis) : axis;

		bool isFirst = (simulatedXInputAxis & bit) == 0;
		simulatedXInputAxis |= bit;

		switch (bit)
		{
			case XInputAxis::leftStickX:
				if (isFirst || axis > std::abs(Input.Gamepad.sThumbLX))
				{
					Input.Gamepad.sThumbLX = workAxis;
				}

				break;

			case XInputAxis::leftStickY:
				if (isFirst || axis > std::abs(Input.Gamepad.sThumbLY))
				{
					Input.Gamepad.sThumbLY = workAxis;
				}

				break;

			case XInputAxis::rightStickX:
				if (isFirst || axis > std::abs(Input.Gamepad.sThumbRX))
				{
					Input.Gamepad.sThumbRX = workAxis;
				}

				break;

			case XInputAxis::rightStickY:
				if (isFirst || axis > std::abs(Input.Gamepad.sThumbRY))
				{
					Input.Gamepad.sThumbRY = workAxis;
				}

				break;

			case XInputAxis::leftTrigger:
				if (isFirst || trigger > Input.Gamepad.bLeftTrigger)
				{
					Input.Gamepad.bLeftTrigger = trigger;
				}

				break;

			case XInputAxis::rightTrigger:
				if (isFirst || trigger > Input.Gamepad.bRightTrigger)
				{
					Input.Gamepad.bRightTrigger = trigger;
				}

				break;

			default:
				throw /* TODO: new ArgumentOutOfRangeException(nameof(options), options, "Invalid XInput axis.")*/;
		}
	}
}

bool Ds4Device::IsOverriddenByModifierSet(InputMapBase& map)
{
	if (Profile.Modifiers.empty())
	{
		return false;
	}

	/*std::list<InputMap> maps = Profile.Modifiers
		.Where(x = > x.IsActive)
		.Select(y = > y.Bindings)
		.Where(x = > x != nullptr)
		.SelectMany(x = > x)
		.Where(x = > !ReferenceEquals(x, map))
		.ToList();*/

	std::list<InputMap*> maps;

	for (InputModifier& mod : Profile.Modifiers)
	{
		if (!mod.IsActive())
		{
			continue;
		}

		for (InputMap& binding : mod.Bindings)
		{
			if (&binding != &map)
			{
				maps.push_back(&binding);
			}
		}
	}

	if ((map.inputType & InputType::button) != 0)
	{
		//if (maps.Any(x =  > (x.inputType & InputType::button) != 0 && (x.InputButtons & map.InputButtons) != 0))
		if (std::any_of(maps.begin(), maps.end(), [&](InputMap* x) -> bool { return (x->inputType & InputType::button) != 0 && (x->InputButtons & map.InputButtons) != 0; }))
		{
			return true;
		}
	}

	if ((map.inputType & InputType::axis) != 0)
	{
		//if (maps.Any(x =  > (x.inputType & InputType::axis) != 0 && (x.InputAxis & map.InputAxis) != 0))
		if (std::any_of(maps.begin(), maps.end(), [&](InputMap* x) -> bool { return (x->inputType & InputType::axis) != 0 && (x->InputAxis & map.InputAxis) != 0; }))
		{
			return true;
		}
	}

	if ((map.inputType & InputType::touchRegion) != 0)
	{
		//if (maps.Any(x =  > (x.inputType & InputType::touchRegion) != 0 && x.InputRegion == map.InputRegion))
		if (std::any_of(maps.begin(), maps.end(), [&](InputMap* x) -> bool { return (x->inputType & InputType::touchRegion) != 0 && x->InputRegion == map.InputRegion; }))
		{
			return true;
		}
	}

	return false;
}

void Ds4Device::RunMap(InputMap& m, InputModifier* modifier)
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
				PressedState state = m.SimulatedState();
				ApplyMap(m, modifier, state, m.IsActive() && ((modifier && modifier->IsActive()) || m.IsToggled) ? 1.0f : 0.0f);
				break;
			}

			case InputType::axis:
			{
				if (m.InputAxis == 0)
				{
					throw /* TODO: new ArgumentNullException(nameof(m.InputAxis))*/;
				}

				for (Ds4Axis_t bit : Ds4Axis_values)
				{
					if ((m.InputAxis & bit) == 0)
					{
						continue;
					}

					InputAxisOptions options = m.GetAxisOptions(bit);

					float analog = Input.GetAxis(m.InputAxis, &options.Polarity);
					options.ApplyDeadZone(analog);

					PressedState state = m.SimulatedState();
					ApplyMap(m, modifier, state, analog);
				}

				break;
			}

			case InputType::touchRegion:
			{
				Ds4TouchRegion region = Profile.TouchRegions[m.InputRegion];

				if (region.Type == +Ds4TouchRegionType::button || m.touchDirection == Direction::none)
				{
					PressedState state = HandleTouchToggle(m, modifier, region.State1);
					ApplyMap(m, modifier, state, Pressable::IsActiveState(state) ? 1.0f : 0.0f);

					state = HandleTouchToggle(m, modifier, region.State2);
					ApplyMap(m, modifier, state, Pressable::IsActiveState(state) ? 1.0f : 0.0f);
				}
				else
				{
					Direction_t direction = m.touchDirection;

					float deadZone = region.GetDeadZone(direction);

					PressedState state = HandleTouchToggle(m, modifier, region.State1);
					float analog = region.GetTouchDelta(Ds4Buttons::touch1, direction, Input.Data.TouchPoint1);

					if (analog < deadZone)
					{
						Pressable::Release(state);
					}

					region.ApplyDeadZone(direction, analog);
					ApplyMap(m, modifier, state, analog);

					state  = HandleTouchToggle(m, modifier, region.State2);
					analog = region.GetTouchDelta(Ds4Buttons::touch2, direction, Input.Data.TouchPoint2);

					if (analog < deadZone)
					{
						Pressable::Release(state);
					}

					region.ApplyDeadZone(direction, analog);
					ApplyMap(m, modifier, state, analog);
				}

				return;
			}

			default:
				throw /* TODO: new ArgumentOutOfRangeException()*/;
		}
	}
}

PressedState Ds4Device::HandleTouchToggle(InputMap& m, InputModifier* modifier, Pressable pressable)
{
	if (m.touchDirection != Direction::none)
	{
		return m.IsToggled ? m.SimulatedState() : pressable.State;
	}

	if (m.RapidFire == true)
	{
		return m.SimulatedState();
	}

	PressedState state = (m.IsToggled || (modifier && modifier->IsActive() == true)) ? pressable.State : m.State;

	if (!Pressable::IsActiveState(state))
	{
		state = m.SimulatedState();
	}

	return state;
}

void Ds4Device::ApplyMap(InputMap& m, InputModifier* modifier, PressedState state, float analog)
{
	switch (m.simulatorType)
	{
		case SimulatorType::input:
			switch (m.outputType)
			{
				case OutputType::xinput:
					if (m.xinputButtons)
					{
						SimulateXInputButton(m.xinputButtons, state);
					}

					if (m.xinputAxes.Axes)
					{
						SimulateXInputAxis(m.xinputAxes, analog);
					}

					break;

				case OutputType::keyboard:
					SimulateKeyboard(m, state);
					break;

					// TODO: AxisOptions thing for mouse
				case OutputType::mouse:
					SimulateMouse(m, state, analog);
					break;

				default:
					throw /* TODO: new ArgumentOutOfRangeException(nameof(m.outputType), m.outputType, "Invalid output type.")*/;
			}

			break;

		case SimulatorType::action:
			if (m.action == +ActionType::none)
			{
				throw /* TODO: new ArgumentNullException(nameof(m.action))*/;
			}

			if (m.IsActive() && (modifier && modifier->IsActive()))
			{
				RunAction(m.action);
			}

			break;

		default:
			throw /* TODO: new ArgumentOutOfRangeException(nameof(m.simulatorType), m.simulatorType, "Invalid map type.")*/;
	}
}

void Ds4Device::SimulateMouse(const InputMap& m, PressedState state, float analog)
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

void Ds4Device::SimulateKeyboard(const InputMap& m, PressedState state)
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

void Ds4Device::RunAction(ActionType action)
{
	switch (action)
	{
		case ActionType::bluetoothDisconnect:
			if (BluetoothConnected())
			{
				DisconnectBluetooth();
				// TODO: Logger.WriteLine(LogLevel.Info, Name, Resources.BluetoothDisconnected);
			}

			break;

		default:
			throw /* TODO: new ArgumentOutOfRangeException(nameof(action), action, "Invalid action type.")*/;
	}
}

void Ds4Device::RunMaps()
{
	simulatedXInputAxis = 0;

	for (InputModifier& modifier : Profile.Modifiers)
	{
		UpdatePressedState(modifier);
	}

	if (Input.TouchChanged)
	{
		UpdateTouchRegions();
	}

	for (InputMap& m : Profile.Bindings)
	{
		UpdateBindingState(m, nullptr);
	}
}

void Ds4Device::RunPersistent()
{
	simulatedXInputAxis = 0;

	for (InputModifier& modifier : Profile.Modifiers)
	{
		if (modifier.IsPersistent())
		{
			UpdatePressedState(modifier);
		}
	}

	for (InputMap& m : Profile.Bindings)
	{
		if (m.IsPersistent())
		{
			UpdateBindingState(m, nullptr);
		}
	}
}

void Ds4Device::UpdateTouchRegions()
{
	Ds4Buttons_t disallow = 0;

	//foreach(Ds4TouchRegion region in Profile.TouchRegions.Values.OrderBy(x = > !x.IsActive(touchMask) && !x.AllowCrossOver))

	std::sort(touchRegions.begin(), touchRegions.end(), [](const Ds4TouchRegion* a, const Ds4TouchRegion* b)
	{
		return (a->IsActive(touchMask) && !a->AllowCrossOver) && !(b->IsActive(touchMask) && !b->AllowCrossOver);
	});

	for (auto& region : touchRegions)
	{
		if ((Input.HeldButtons & touchMask) == 0)
		{
			region->SetInactive(touchMask);
			continue;
		}

		UpdateTouchRegion(*region, /* TODO */ nullptr, Ds4Buttons::touch1, Input.Data.TouchPoint1, disallow);
		UpdateTouchRegion(*region, /* TODO */ nullptr, Ds4Buttons::touch2, Input.Data.TouchPoint2, disallow);
	}
}

void Ds4Device::UpdateTouchRegion(Ds4TouchRegion& region, InputModifier* modifier, Ds4Buttons_t sender, Ds4Vector2& point, Ds4Buttons_t& disallow)
{
	if ((disallow & sender) == 0 && (Input.HeldButtons & sender) != 0)
	{
		if (region.IsInRegion(sender, point) && modifier)
		{
			if (modifier->IsActive() == false)
			{
				UpdatePressedState(*modifier);
			}

			if (modifier && modifier->IsActive() != false)
			{
				region.SetActive(sender, point);

				if (!region.AllowCrossOver)
				{
					disallow |= sender;
				}

				return;
			}
		}
	}

	region.SetInactive(sender);
}

void Ds4Device::UpdatePressedStateImpl(InputMapBase& instance, const std::function<void()>& press, const std::function<void()>& release)
{
	if (IsOverriddenByModifierSet(instance))
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
				if ((instance.InputButtons & Input.HeldButtons) == instance.InputButtons)
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
				if (instance.InputAxis == 0)
				{
					throw /* TODO: new ArgumentNullException(nameof(instance.InputAxis))*/;
				}

				const gsl::span<const Ds4Axis_t> s(Ds4Axis_values);

				size_t target = std::count_if(s.begin(), s.end(), [&](Ds4Axis_t x) -> bool { return (x & instance.InputAxis) != 0; });
				size_t count  = 0;

				for (Ds4Axis_t bit : Ds4Axis_values)
				{
					if ((instance.InputAxis & bit) == 0)
					{
						continue;
					}

					InputAxisOptions options = instance.GetAxisOptions(bit);

					// ReSharper disable once PossibleInvalidOperationException
					float axis = Input.GetAxis(instance.InputAxis, &options.Polarity);

					if (axis >= options.deadZone)
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
				auto it = Profile.TouchRegions.find(instance.InputRegion);
				if (it == Profile.TouchRegions.end())
				{
					break;
				}

				auto& region = it->second;

				if (region.IsActive(touchMask))
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

void Ds4Device::UpdatePressedState(InputModifier& modifier)
{
	const auto a = [&]() -> void
	{
		modifier.Press();
	};

	const auto b = [&]() -> void
	{
		modifier.Release();
	};

	UpdatePressedStateImpl(modifier, a, b);

	if (modifier.Bindings.empty())
	{
		return;
	}

	for (InputMap& bind : modifier.Bindings)
	{
		UpdateBindingState(bind, &modifier);
	}
}

void Ds4Device::UpdatePressedState(InputMap& map, InputModifier* modifier)
{
	const auto a = [&]() -> void
	{
		map.Press(modifier);
	};

	const auto b = [&]() -> void
	{
		map.Release();
	};

	UpdatePressedStateImpl(map, a, b);
}

void Ds4Device::UpdateBindingState(InputMap& m, InputModifier* modifier)
{
	if (modifier != nullptr)
	{
		if (m.Toggle != true)
		{
			if (!modifier->IsActive())
			{
				m.Release();
				RunMap(m, modifier);
				return;
			}
		}
	}

	switch (m.inputType)
	{
		case InputType::touchRegion:
		{
			auto it = Profile.TouchRegions.find(m.InputRegion);
			if (it == Profile.TouchRegions.end())
			{
				break;
			}

			auto& region = it->second;

			if (m.touchDirection == Direction::none)
			{
				if (region.IsActive(touchMask))
				{
					m.Press(modifier);
				}
				else
				{
					m.Release();
				}
			}

			break;
		}

		default:
			UpdatePressedState(m, modifier);
			break;
	}

	RunMap(m, modifier);
}
