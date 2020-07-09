#include "pch.h"

#include <ViGEm/Client.h>

#include "ViGEmTarget.h"
#include "ViGEmDriver.h"

namespace vigem
{
	XInputTarget::XInputTarget(Driver* parent)
		: parent(parent)
	{
		target = vigem_target_x360_alloc();

		if (target == nullptr)
		{
			throw std::runtime_error("Unable to allocate XInput ViGEm target.");
		}
	}

	XInputTarget::XInputTarget(XInputTarget&& rhs) noexcept
		: parent(rhs.parent),
		  target(rhs.target),
		  connected_(rhs.connected_)
	{
		rhs.unregisterNotification();
		rhs.target = nullptr;
		rhs.connected_ = false;
		registerNotification();
	}

	XInputTarget::~XInputTarget()
	{
		close();
	}

	VIGEM_ERROR XInputTarget::connect()
	{
		if (connected())
		{
			return VIGEM_ERROR_NONE;
		}

		auto guard = parent->lock();

		const VIGEM_ERROR result = vigem_target_add(parent->client, target);

		if (!VIGEM_SUCCESS(result))
		{
			return result;
		}

		connected_ = true;

		return registerNotification();
	}

	bool XInputTarget::connected() const
	{
		return connected_;
	}

	VIGEM_ERROR XInputTarget::disconnect()
	{
		if (!connected())
		{
			return VIGEM_ERROR_NONE;
		}

		auto guard = parent->lock();
		const VIGEM_ERROR result = vigem_target_remove(parent->client, target);

		if (VIGEM_SUCCESS(result))
		{
			connected_ = false;
			unregisterNotification();
		}

		return result;
	}

	void XInputTarget::update(const XInputGamepad& data) const
	{
		auto guard = parent->lock();
		vigem_target_x360_update(parent->client, this->target,
		                         *reinterpret_cast<const XUSB_REPORT*>(&data));
	}

	void XInputTarget::close()
	{
		auto guard = parent->lock();
		disconnect();

		if (target)
		{
			vigem_target_free(target);
			target = nullptr;
		}
	}

	VIGEM_ERROR XInputTarget::registerNotification()
	{
		const VIGEM_ERROR result = vigem_target_x360_register_notification(parent->client, target,
		                                                                   &XInputTarget::raiseEvent,
		                                                                   this);

		if (!VIGEM_SUCCESS(result))
		{
			disconnect();
		}

		return result;
	}

	void XInputTarget::unregisterNotification()
	{
		vigem_target_x360_unregister_notification(target);
	}

	void XInputTarget::raiseEvent(PVIGEM_CLIENT client, PVIGEM_TARGET target,
	                              uint8_t largeMotor, uint8_t smallMotor, uint8_t ledNumber,
	                              LPVOID userData)
	{
		auto* xinput_target = static_cast<XInputTarget*>(userData);

		if (xinput_target->parent->client == client)
		{
			xinput_target->notification.invoke(xinput_target, largeMotor, smallMotor, ledNumber);
		}
	}
}
