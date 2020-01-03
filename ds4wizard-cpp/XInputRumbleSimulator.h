#pragma once
#include <memory>
#include <Xinput.h>

#include "ISimulator.h"
#include "ViGEmTarget.h"
#include "Event.h"

class XInputRumbleSimulator : public ISimulator
{
	XINPUT_VIBRATION xinputVibration;
	EventToken xinputNotification;

public:
	std::shared_ptr<vigem::XInputTarget> xinputTarget;

	explicit XInputRumbleSimulator(InputSimulator* parent);
	~XInputRumbleSimulator() override = default;

	void update(float deltaTime) override;

private:
	void onActivate(float deltaTime) override;
	void onDeactivate(float deltaTime) override;
};
