#include "stdafx.h"
#include "Latency.h"

#include <chrono>

using namespace std::chrono;

Latency::Latency()
	: average_(1000ms)
{
}

void Latency::start()
{
	stopwatch.start();
}

Stopwatch::Duration Latency::stop()
{
	if (!stopwatch.running())
	{
		return lastValue_;
	}

	lastValue_ = stopwatch.stop();

	average_.push(lastValue_);

	if (lastValue_ > peak_)
	{
		peak_ = lastValue_;
	}

	return lastValue_;
}

Stopwatch::Duration Latency::elapsed() const
{
	return stopwatch.elapsed();
}

bool Latency::running() const
{
	return stopwatch.running();
}

Stopwatch::Duration Latency::lastValue() const
{
	return lastValue_;
}

Stopwatch::Duration Latency::peak()
{
	if (stopwatch.running())
	{
		if (stopwatch.elapsed() > peak_)
		{
			peak_ = stopwatch.elapsed();
		}
	}

	return peak_;
}

Stopwatch::Duration Latency::average()
{
	return average_.value();
}

void Latency::resetPeak()
{
	peak_ = 0ms;
}
