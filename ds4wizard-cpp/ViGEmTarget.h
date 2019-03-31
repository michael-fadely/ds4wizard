#pragma once

#include <unordered_map>
#include <mutex>

#include <ViGEm/Client.h>

#include "Event.h"

namespace vigem
{
	class Driver;

	class XInputTarget
	{
		friend class Driver;

		Driver const* parent;
		PVIGEM_TARGET target = nullptr;
		bool connected_ = false;

		static std::recursive_mutex static_mutex;
		static std::unordered_map<PVIGEM_TARGET, XInputTarget*> targets;

	public:
		// large motor, small motor, led number
		Event<XInputTarget, uint8_t, uint8_t, uint8_t> notification;

		/**
		 * \brief Explicitly disallow default construction.
		 */
		XInputTarget() = delete;

		/**
		 * \brief Explicitly disallow copying.
		 */
		XInputTarget(const XInputTarget&) = delete;

		/**
		 * \brief Constructs an XInput endpoint using the given driver instance.
		 * \param parent \c Driver instance to use for configuring the endpoint.
		 */
		explicit XInputTarget(Driver* parent);
		XInputTarget(XInputTarget&& rhs) noexcept;
		~XInputTarget();

		/**
		 * \brief Connects a virtual XInput device to the system and registers event handlers.
		 * \return \c VIGEM_ERROR_NONE on success.
		 */
		VIGEM_ERROR connect();

		/**
		 * \brief Specifies connected state of the virtual XInput device to the system.
		 * \return \c true if the device is connected.
		 */
		bool connected() const;

		/**
		 * \brief Disconnects the virtual XInput device from the system and unregisters event handlers.
		 * \return \c VIGEM_ERROR_NONE on success.
		 */
		VIGEM_ERROR disconnect();

		/**
		 * \brief Writes input data to the virtual XInput device.
		 * \param data The data to write to the virtual XInput device.
		 */
		void update(const XInputGamepad& data) const;

		/**
		 * \brief Explicitly disallow copying.
		 */
		XInputTarget& operator=(const XInputTarget&) = delete;

	private:
		void close();
		static void raiseEvent(PVIGEM_CLIENT client, PVIGEM_TARGET target, uint8_t largeMotor, uint8_t smallMotor, uint8_t ledNumber);
	};
}
