#include "pch.h"
#include "Stopwatch.h"

Stopwatch::Stopwatch(bool start_now)
{
	if (start_now)
	{
		start();
	}
}

void Stopwatch::start()
{
	running_ = true;
	start_time_ = Clock::now();
}

Stopwatch::Duration Stopwatch::stop()
{
	end_time_ = Clock::now();
	running_ = false;
	return elapsed();
}

Stopwatch::Duration Stopwatch::elapsed() const
{
	return (running_ ? Clock::now() : end_time_) - start_time_;
}

bool Stopwatch::running() const
{
	return running_;
}

Stopwatch::TimePoint Stopwatch::start_time() const
{
	return start_time_;
}

Stopwatch::TimePoint Stopwatch::end_time() const
{
	return end_time_;
}
