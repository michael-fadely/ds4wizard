#pragma once

#include "Stopwatch.h"
#include "average.h"

class Latency
{
	Stopwatch stopwatch;
	Stopwatch::Duration lastValue_ {};
	Stopwatch::Duration peak_ {};
	timed_average<Stopwatch::Duration> average_;

public:
	Latency();

	void start();
	Stopwatch::Duration stop();
	Stopwatch::Duration elapsed() const;
	bool running() const;

	Stopwatch::Duration lastValue() const;
	Stopwatch::Duration peak();
	Stopwatch::Duration average();
	void resetPeak();
};
