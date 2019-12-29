#pragma once

class InputSimulator;

enum class SimulatorState
{
	inactive,
	active
};

class ISimulator
{
public:
	SimulatorState state;
	InputSimulator* parent;

	explicit ISimulator(InputSimulator* parent);
	virtual ~ISimulator() = default;

	virtual void activate(float deltaTime);
	virtual void update(float deltaTime) = 0;
	virtual void deactivate(float deltaTime);

private:
	virtual void onActivate(float deltaTime) = 0;
	virtual void onDeactivate(float deltaTime) = 0;
};
