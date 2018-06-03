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

using namespace std::chrono;

bool Ds4Device::DisconnectOnIdle() const
{
	return Settings.UseProfileIdle ? Profile.Idle.Disconnect : Settings.Idle.Disconnect;
}

DeviceIdleOptions::clock::duration Ds4Device::IdleTimeout() const
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
	return bluetoothDevice->is_open();
}

bool Ds4Device::UsbConnected()
{
	lock(sync);
	return usbDevice->is_open();
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

	macAddress << std::hex << std::setw(2) << std::setfill('0') << device.serial[0];

	for (size_t i = 1; i < device.serial.size(); ++i)
	{
		macAddress << ':' << std::hex << std::setw(2) << std::setfill('0') << device.serial[i];
	}

	MacAddress = macAddress.str();
	SafeMacAddress = MacAddress;

	SafeMacAddress.erase(std::remove(SafeMacAddress.begin(), SafeMacAddress.end(), ':'), SafeMacAddress.end());

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

	if (!Program::ProfileCache.GetSettings(MacAddress, Settings))
	{
		Settings = {};
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

	DeviceProfile profile = Program::ProfileCache.GetProfile(Settings.Profile);
	if (profile == nullptr)
	{
		Settings.Profile = nullptr;
		Profile = new DeviceProfile(DeviceProfile.Default);
	}
	else
	{
		Profile = profile;
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
		l.Color = Ds4AutoLightColor.GetColor(out colorIndex);
	}

	activeLight = new Ds4LightOptions(l);

	if (usbDevice != nullptr && (!UsbConnected() || usbDevice->is_exclusive() != Profile.ExclusiveMode))
	{
		CloseUsbDevice();
		OpenUsbDevice(usbDevice);
	}

	if (bluetoothDevice != nullptr && (!BluetoothConnected() || bluetoothDevice->is_exclusive() != Profile.ExclusiveMode))
	{
		CloseBluetoothDevice();
		OpenBluetoothDevice(bluetoothDevice);
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
		//Logger.WriteLine(LogLevel.Warning, Resources.ScpVBusMissing); // TODO
		return false;
	}

	HANDLE handle = CreateFile(info->path.c_str(), GENERIC_READ | GENERIC_WRITE,
	                           FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);

	if (handle == INVALID_HANDLE_VALUE || handle == nullptr)
	{
		//Logger.WriteLine(LogLevel.Warning, Resources.ScpVBusOpenFailed); // TODO
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
		//Logger.WriteLine(LogLevel.Warning, Resources.XInputCreateFailed); // TODO
		return false;
	}

	realXInputIndex = index;
	return true;
}

void Ds4Device::ScpDeviceClose()
{
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
	Ds4AutoLightColor.ReleaseColor(colorIndex);
	colorIndex = -1;
}

void Ds4Device::OnProfileChanged(std::string newName)
{
	lock(sync);
	Settings.Profile = newName.empty() ? std::string() : newName;
	SaveSettings();
	ApplyProfile();
}

void Ds4Device::Close()
{
	lock(sync);
	CloseUsbDevice();
	CloseBluetoothDevice();
	scpDevice->Disconnect(realXInputIndex);
	scpDevice->Close();
	ReleaseAutoColor();
}

void Ds4Device::CloseBluetoothDevice()
{
	lock(sync);
	bluetoothDevice->close();
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
	usbDevice->close();
	idleTime.start();
}

bool Ds4Device::OpenDevice(std::unique_ptr<hid::HidInstance>& device, bool exclusive)
{
	if (device->open((exclusive ? hid::HidOpenFlags::exclusive : 0) | hid::HidOpenFlags::async))
	{
		return true;
	}

	return exclusive && device->open(hid::HidOpenFlags::async);
}

void Ds4Device::OpenBluetoothDevice(std::unique_ptr<hid::HidInstance>& device)
{
	if (device == nullptr)
	{
		return;
	}

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

		if (Profile.ExclusiveMode && !device->is_exclusive())
		{
			//Logger.WriteLine(LogLevel.Warning, Name, Resources.BluetoothExclusiveOpenFailed); // TODO
		}
		else
		{
			//Logger.WriteLine(LogLevel.Info, Name, Resources.BluetoothConnected); // TODO
		}

		bluetoothDevice = std::move(device);

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

void Ds4Device::OpenUsbDevice(std::unique_ptr<hid::HidInstance>& device)
{
	if (device == nullptr)
	{
		return;
	}

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

		if (Profile.ExclusiveMode && !device->is_exclusive())
		{
			//Logger.WriteLine(LogLevel.Warning, Name, Resources.UsbExclusiveOpenFailed); // TODO
		}
		else
		{
			//Logger.WriteLine(LogLevel.Info, Name, Resources.UsbConnected); // TODO
		}

		usbDevice = std::move(device);
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

	ConnectionType preferredConnection = ConnectionType::usb;

	preferredConnection = Program::settings.preferredConnection;

	// HACK: make this class manage the light state
	Output.LightColor = activeLight.Color;

	// HACK: see above
	if (activeLight.IdleFade)
	{
		Ds4LightOptions l = Settings.UseProfileLight ? Profile.Light : Settings.Light;
		double m = IsIdle ? 1.0 : clamp(duration_cast<milliseconds>(idleTime.elapsed()).count() / static_cast<double>(duration_cast<milliseconds>(IdleTimeout()).count()), 0.0, 1.0);

		Output.LightColor = Ds4Color::Lerp(l.Color, fadeColor, static_cast<float>(m));
	}

	bool charging = Charging;
	uint8_t battery = Battery();

	// cache
	bool usb       = UsbConnected();
	bool bluetooth = BluetoothConnected();

	bool useUsb       = usb && (preferredConnection == ConnectionType::usb || !bluetooth);
	bool useBluetooth = bluetooth && (preferredConnection == ConnectionType::bluetooth|| !usb);
	DataReceived      = false;

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

	float lx = Input.GetAxis(Ds4Axis::LeftStickX, nullptr);
	float ly = Input.GetAxis(Ds4Axis::LeftStickY, nullptr);
	auto ls   = static_cast<float>(std::sqrt(lx * lx + ly * ly));

	float rx = Input.GetAxis(Ds4Axis::RightStickX, nullptr);
	float ry = Input.GetAxis(Ds4Axis::RightStickY, nullptr);
	auto rs   = static_cast<float>(std::sqrt(rx * rx + ry * ry));

	// TODO: gyro/accel
	if (Input.ButtonsChanged || Input.HeldButtons != 0
	    || ls >= 0.25f || rs >= 0.25f)
	{
		idleTime.start();
	}
	else if (DisconnectOnIdle() && useBluetooth && IsIdle())
	{
		DisconnectBluetooth();
		//Logger.WriteLine(LogLevel.Info, Name, std::string.Format(Resources.IdleDisconnect, IdleTimeout)); // TODO
	}

	if (DataReceived)
	{
		RunMaps();

		if (Latency.elapsed() >= 5ms)
		{
			// TODO: configurable latency target & light flash (although that increases latency on send)
			//Debug.WriteLine($"{Latency.Elapsed.TotalMilliseconds} ms"); // TODO
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

	while (Connected)
	{
		lock(sync);
		Run();
	}

	Close();
	OnDeviceClosed();
}

void Ds4Device::OnDeviceClosed()
{
	//DeviceClosed.Invoke(this, EventArgs.Empty); // TODO
}

void Ds4Device::Start()
{
	if (ioThread == nullptr)
	{
		ioThread = std::make_unique<std::thread>(&ControllerThread);
	}
}

void Ds4Device::OnBatteryLevelChanged()
{
	//BatteryLevelChanged.Invoke(this, EventArgs.Empty); // TODO
}
