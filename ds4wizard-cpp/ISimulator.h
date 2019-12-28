#pragma once

enum class SimulatorState
{
	inactive,
	active
};

class ISimulator
{
public:
	virtual ~ISimulator() = default;
	SimulatorState state;

	virtual void activate(float deltaTime);
	virtual void update(float deltaTime) = 0;
	virtual void deactivate(float deltaTime);

private:
	virtual void onActivate(float deltaTime) = 0;
	virtual void onDeactivate(float deltaTime) = 0;
};
