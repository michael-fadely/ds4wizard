#include "pch.h"
#include "ViGEmTarget.h"
#include "XInputRumbleSimulator.h"

XInputRumbleSimulator::XInputRumbleSimulator(InputSimulator* parent)
	: ISimulator(parent),
	  xinputVibration({})
{
}

void XInputRumbleSimulator::update(float deltaTime)
{
	parent->setRumble(static_cast<float>(xinputVibration.wLeftMotorSpeed >> 8) / 255.0f,
	                  static_cast<float>(xinputVibration.wRightMotorSpeed >> 8) / 255.0f);
}

void XInputRumbleSimulator::onActivate(float deltaTime)
{
	if (xinputTarget)
	{
		this->xinputNotification = xinputTarget->notification.add(
			[&](auto sender, auto large, auto small, auto led) -> void
			{
				this->xinputVibration.wLeftMotorSpeed  = (large << 8) | large;
				this->xinputVibration.wRightMotorSpeed = (small << 8) | small;
			});
	}
}

void XInputRumbleSimulator::onDeactivate(float deltaTime)
{
	if (xinputTarget)
	{
		xinputTarget->notification.remove(this->xinputNotification);
	}
}
