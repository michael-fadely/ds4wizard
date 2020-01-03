#include "pch.h"
#include "ISimulator.h"

ISimulator::ISimulator(InputSimulator* parent)
	: state(SimulatorState::inactive),
	  parent(parent)
{
}

void ISimulator::activate(float deltaTime)
{
	if (state == SimulatorState::inactive)
	{
		state = SimulatorState::active;
		onActivate(deltaTime);
	}
}

void ISimulator::deactivate(float deltaTime)
{
	if (state == SimulatorState::active)
	{
		state = SimulatorState::inactive;
		onDeactivate(deltaTime);
	}
}
