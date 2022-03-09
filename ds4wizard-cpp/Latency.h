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
	[[nodiscard]] Stopwatch::Duration elapsed() const;
	[[nodiscard]] bool running() const;

	[[nodiscard]] Stopwatch::Duration lastValue() const;
	[[nodiscard]] Stopwatch::Duration peak();
	[[nodiscard]] Stopwatch::Duration average();
	void resetPeak();
};
