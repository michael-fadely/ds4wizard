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

	void activate(float deltaTime);
	virtual void update(float deltaTime) = 0;
	void deactivate(float deltaTime);

private:
	virtual void onActivate(float deltaTime) {}
	virtual void onDeactivate(float deltaTime) {}
};
