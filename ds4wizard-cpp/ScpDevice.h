#pragma once

#include <array>
#include <cstdint>
#include <mutex>

#include "XInputGamepad.h"

enum class VBusStatus
{
	Success            = 0,
	NotConnected       = 0x90000,
	IndexOutOfRange    = 0x90001,
	IoctlRequestFailed = 0x90002,
	InvalidStateInfo   = 0x90003,
	DeviceNotReady     = 0x90004,
};

struct ScpVibration
{
	uint8_t LeftMotor, RightMotor;
};

// {F679F562-3164-42CE-A4DB-E7DDBE723909}
DEFINE_GUID(GUID_DEVINTERFACE_SCPVBUS, 0xf679f562, 0x3164, 0x42ce, 0xa4, 0xdb, 0xe7, 0xdd, 0xbe, 0x72, 0x39, 0x9);

class ScpDevice
{
	static std::mutex portLock_lock;

	// HACK: This is prone to failure. Probably better to use integers.
	/// <summary>
	/// Emualted ports currently in use.
	/// </summary>
	inline static std::array<bool, 4> portsUsed =
	{
		false, false, false, false
	};

public:
	static int GetFreePort();

private:
	std::array<uint8_t, 4> leds {};

	std::array<XInputGamepad, 4> gamepads {};
	std::array<ScpVibration, 4> vibration {};

	std::array<uint8_t, 10> readBuffer {};
	std::array<uint8_t, 28> writeBuffer {};

	HANDLE handle = nullptr;

public:
	std::array<short, 4> Version {};

	/// <summary>
	/// Initializes the ScpVBus device using the given handle.
	/// </summary>
	/// <param name="handle">A valid handle to the ScpVBus device.</param>
	ScpDevice(HANDLE handle);

private:
	static int GetDriverVersion(uint64_t& version);

public:
	/// <summary>
	/// Closes the handle to the ScpVBus device.
	/// </summary>
	void Close();

	/// <summary>
	/// Connects the specified emulated XInput device.
	/// </summary>
	/// <param name="userIndex">The index to connect.</param>
	/// <returns><value>true</value> on success.</returns>
	bool Connect(int userIndex);

	/// <summary>
	/// Disconnect the specified emulated XInput device.
	/// </summary>
	/// <param name="userIndex">The index to disconnect. -1 disconnects all devices.</param>
	/// <param name="force">Force disconnect.</param>
	/// <returns><value>true</value> on success.</returns>
	bool Disconnect(int userIndex, bool force = true);

	/// <summary>
	/// Sets the current emulated XInput state and reads any pending data.
	/// </summary>
	/// <param name="userIndex">The index of the emualted device.</param>
	/// <param name="gamepad">A gamepad structure containing the data to set.</param>
	/// <returns><value>VBusStatus::Success</value> on success.</returns>
	/// <seealso cref="VBusStatus"/>
	VBusStatus SyncState(int userIndex, const XInputGamepad& gamepad);

	/// <summary>
	/// Sets the emulated XInput state and reads any pending data.
	/// </summary>
	/// <param name="userIndex">The index of the emualted device.</param>
	/// <returns><value>VBusStatus::Success</value> on success.</returns>
	/// <seealso cref="VBusStatus"/>
	VBusStatus SyncState(int userIndex);

	/// <summary>
	/// Gets the last stored emulated XInput device state.
	/// </summary>
	/// <param name="userIndex">The emulated device index.</param>
	/// <returns>The last stored emulated XInput device state.</returns>
	/// <seealso cref="XInputGamepad"/>
	XInputGamepad GetGamepad(int userIndex);

	/// <summary>
	/// Gets the last stored emulated XInput vibration state.
	/// </summary>
	/// <param name="userIndex">The emulated device index.</param>
	/// <param name="leftMotor">The variable to receive the left (large) motor rumble state.</param>
	/// <param name="rightMotor">The variable to receive the right (small) motor rumble state.</param>
	void GetVibration(int userIndex, uint8_t& leftMotor, uint8_t& rightMotor);

	/// <summary>
	/// Gets the last stored emulated LED ID.
	/// </summary>
	/// <param name="userIndex">The emulated device index.</param>
	/// <returns>The last stored emulated LED ID.</returns>
	uint8_t GetLed(int userIndex);
};
