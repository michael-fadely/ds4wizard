#pragma once

#include <queue>
#include <cstdint>

#include "ISimulator.h"
#include "Stopwatch.h"

enum class RumbleSequenceBlending
{
	none,
	linear
};

struct RumbleSequenceElement
{
	RumbleSequenceBlending blending = RumbleSequenceBlending::none;
	int64_t durationMilliseconds = 0;
	float leftMotor = 0.0f;
	float rightMotor = 0.0f;
};

class InputSimulator;

class RumbleSequence : public ISimulator
{
	std::queue<RumbleSequenceElement> sequence;
	std::optional<RumbleSequenceElement> currentElement;
	Stopwatch stopwatch;

public:
	explicit RumbleSequence(InputSimulator* parent);

	void update(float deltaTime) override;
	void add(const RumbleSequenceElement& element);
};

class RumbleTimer : public ISimulator
{
	Stopwatch stopwatch;

public:
	RumbleTimer(InputSimulator* parent, Stopwatch::Duration duration,
	            float left, float right);

	Stopwatch::Duration duration;
	float left;
	float right;

	void reset();

	void update(float deltaTime) override;
	void onActivate(float deltaTime) override;
};
