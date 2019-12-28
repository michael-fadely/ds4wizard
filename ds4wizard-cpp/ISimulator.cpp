#include "pch.h"
#include "ISimulator.h"

void ISimulator::activate(float deltaTime)
{
	onActivate(deltaTime);
}

void ISimulator::deactivate(float deltaTime)
{
	onDeactivate(deltaTime);
}
