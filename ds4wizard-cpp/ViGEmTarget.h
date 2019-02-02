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

		XInputTarget() = delete;
		explicit XInputTarget(Driver* parent);
		XInputTarget(const XInputTarget&) = delete;
		XInputTarget(XInputTarget&& rhs) noexcept;
		~XInputTarget();

		VIGEM_ERROR connect();
		bool connected() const;
		VIGEM_ERROR disconnect();

		void update(const XInputGamepad& data) const;

		XInputTarget& operator=(const XInputTarget&) = delete;

	private:
		void close();
		static void raiseEvent(PVIGEM_CLIENT client, PVIGEM_TARGET target, uint8_t largeMotor, uint8_t smallMotor, uint8_t ledNumber);
	};
}
