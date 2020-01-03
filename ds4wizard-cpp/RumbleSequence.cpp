#include "pch.h"
#include <chrono>
#include "RumbleSequence.h"

using namespace std::chrono;

RumbleSequence::RumbleSequence(InputSimulator* parent)
	: ISimulator(parent),
	  currentElement(std::nullopt)
{
}

void RumbleSequence::update(float deltaTime)
{
	if (sequence.empty())
	{
		deactivate(deltaTime);
		return;
	}

	const auto elapsed = stopwatch.elapsed();

	if (!currentElement.has_value() ||
	    elapsed >= milliseconds(currentElement->durationMilliseconds))
	{
		stopwatch.start();
		currentElement = sequence.front();
		sequence.pop();
	}

	switch (currentElement.value().blending)
	{
		case RumbleSequenceBlending::none:
			parent->setRumble(currentElement->leftMotor, currentElement->rightMotor);
			break;

		case RumbleSequenceBlending::linear:
		{
			float left  = 0.0f;
			float right = 0.0f;

			if (!sequence.empty())
			{
				const auto& front = sequence.front();

				left  = front.leftMotor;
				right = front.rightMotor;
			}

			int64_t elapsedMilliseconds = duration_cast<milliseconds>(elapsed).count();
			double f = static_cast<double>(elapsedMilliseconds) / static_cast<double>(currentElement->durationMilliseconds);

			left  = gmath::lerp(currentElement->leftMotor, left, f);
			right = gmath::lerp(currentElement->rightMotor, right, f);

			parent->setRumble(left, right);
			break;
		}

		default:
			break;
	}
}

void RumbleSequence::add(const RumbleSequenceElement& element)
{
	sequence.push(element);
}

RumbleTimer::RumbleTimer(InputSimulator* parent, Stopwatch::Duration duration, float left, float right)
	: ISimulator(parent),
	  duration(duration),
	  left(left),
	  right(right)
{
}

void RumbleTimer::reset()
{
	stopwatch.start();
}

void RumbleTimer::update(float deltaTime)
{
	if (stopwatch.elapsed() >= duration)
	{
		deactivate(deltaTime);
		return;
	}

	parent->setRumble(left, right);
}

void RumbleTimer::onActivate(float deltaTime)
{
	reset();
}
