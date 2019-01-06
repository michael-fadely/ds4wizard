#pragma once

#include <array>
#include <cstdint>
#include <mutex>

#include <hid_handle.h>

#include "XInputGamepad.h"

// {F679F562-3164-42CE-A4DB-E7DDBE723909}
const GUID GUID_DEVINTERFACE_SCPVBUS = { 0xf679f562, 0x3164, 0x42ce, { 0xa4, 0xdb, 0xe7, 0xdd, 0xbe, 0x72, 0x39, 0x9 } };

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
	uint8_t leftMotor;
	uint8_t rightMotor;
};

class ScpDevice
{
	inline static std::recursive_mutex portLock_lock;

	// HACK: This is prone to failure. Probably better to use integers.
	/**
	 * \brief 
	 * Emualted ports currently in use.
	 */
	inline static std::array<bool, 4> portsUsed =
	{
		false, false, false, false
	};

public:
	static ptrdiff_t getFreePort();

private:
	std::array<uint8_t, 4> leds {};

	std::array<XInputGamepad, 4> gamepads {};
	std::array<ScpVibration, 4> vibration {};

	std::array<uint8_t, 10> readBuffer {};
	std::array<uint8_t, 28> writeBuffer {};

	Handle handle = Handle(nullptr, true);

public:
	std::array<short, 4> driverVersion {};

	/**
	 * \brief 
	 * Initializes the ScpVBus device using the given handle.
	 * \param handle A valid handle to the ScpVBus device.
	 */
	explicit ScpDevice(Handle&& handle);

	~ScpDevice();

private:
	static int getDriverVersion(uint64_t& version);

public:
	/**
	 * \brief 
	 * Closes the handle to the ScpVBus device.
	 */
	void close();

	/**
	 * \brief 
	 * Connects the specified emulated XInput device.
	 * \param userIndex The index to connect.
	 * \return \c true on success.
	 */
	bool connect(int userIndex);

	/**
	 * \brief 
	 * Disconnect the specified emulated XInput device.
	 * \param userIndex The index to disconnect.
	 * \param force Force disconnect.
	 * \return \c true on success.
	 */
	bool disconnect(int userIndex, bool force = true);

	/**
	 * \brief
	 * Disconnects all emulated XInput devices managed by the SCP driver.
	 * \param force Forcefully disconnect the devices.
	 * \return \c true on success.
	 */
	bool disconnectAll(bool force = true);

	/**
	 * \brief 
	 * Sets the current emulated XInput state and reads any pending data.
	 * \param userIndex The index of the emualted device.
	 * \param gamepad A gamepad structure containing the data to set.
	 * \return \c VBusStatus::Success on success.
	 * \sa VBusStatus
	 */
	VBusStatus syncState(int userIndex, const XInputGamepad& gamepad);

	/**
	 * \brief 
	 * Sets the emulated XInput state and reads any pending data.
	 * \param userIndex The index of the emualted device.
	 * \return \c VBusStatus::Success on success.
	 * \sa VBusStatus
	 */
	VBusStatus syncState(int userIndex);

	/**
	 * \brief 
	 * Gets the last stored emulated XInput device state.
	 * \param userIndex The emulated device index.
	 * \return The last stored emulated XInput device state.
	 * \sa XInputGamepad
	 */
	XInputGamepad getGamepad(int userIndex);

	/**
	 * \brief 
	 * Gets the last stored emulated XInput vibration state.
	 * \param userIndex The emulated device index.
	 * \param leftMotor The variable to receive the left (large) motor rumble state.
	 * \param rightMotor The variable to receive the right (small) motor rumble state.
	 */
	void getVibration(int userIndex, uint8_t& leftMotor, uint8_t& rightMotor);

	/**
	 * \brief 
	 * Gets the last stored emulated LED ID.
	 * \param userIndex The emulated device index.
	 * \return The last stored emulated LED ID.
	 */
	uint8_t getLed(int userIndex);

private:
	static void validateUserIndex(int userIndex);
	bool disconnect_impl(int userIndex, bool force);
	VBusStatus syncStateImpl(int userIndex);
};
