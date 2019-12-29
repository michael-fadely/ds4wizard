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
	uint8_t leftMotor = 0;
	uint8_t rightMotor = 0;
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
	            uint8_t left, uint8_t right);

	Stopwatch::Duration duration;
	uint8_t left;
	uint8_t right;

	void reset();

	void update(float deltaTime) override;
	void onActivate(float deltaTime) override;
};
