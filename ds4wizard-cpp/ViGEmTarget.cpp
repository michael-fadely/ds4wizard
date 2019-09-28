#include "pch.h"

#include <ViGEm/Client.h>

#include "ViGEmTarget.h"
#include "ViGEmDriver.h"

namespace vigem
{
	std::recursive_mutex XInputTarget::target_mutex;
	std::unordered_map<PVIGEM_TARGET, XInputTarget*> XInputTarget::targets;

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
		rhs.target = nullptr;
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
		VIGEM_ERROR result = vigem_target_add(parent->client, target);

		if (!VIGEM_SUCCESS(result))
		{
			return result;
		}

		connected_ = true;

		result = vigem_target_x360_register_notification(parent->client, target, &XInputTarget::raiseEvent);

		if (!VIGEM_SUCCESS(result))
		{
			disconnect();
		}
		else
		{
			target_mutex.lock();
			targets[target] = this;
			target_mutex.unlock();
		}

		return result;
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

			vigem_target_x360_unregister_notification(target);

			target_mutex.lock();
			targets.erase(target);
			target_mutex.unlock();
		}

		return result;
	}

	void XInputTarget::update(const XInputGamepad& data) const
	{
		auto guard = parent->lock();
		vigem_target_x360_update(parent->client, this->target, *reinterpret_cast<const XUSB_REPORT*>(&data));
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

	void XInputTarget::raiseEvent(PVIGEM_CLIENT client, PVIGEM_TARGET target, uint8_t largeMotor, uint8_t smallMotor, uint8_t ledNumber)
	{
		target_mutex.lock();
		XInputTarget* xtarget = targets[target];
		target_mutex.unlock();

		// HACK: this is occasionally nullptr when connecting the controller via bluetooth; this is not a good fix
		if (!xtarget)
		{
			return;
		}

		if (xtarget->parent->client == client)
		{
			xtarget->notification.invoke(xtarget, largeMotor, smallMotor, ledNumber);
		}
	}
}
