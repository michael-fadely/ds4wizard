#include "stdafx.h"
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
	start_time_ = clock::now();
}

Stopwatch::clock::duration Stopwatch::stop()
{
	end_time_ = clock::now();
	running_ = false;
	return elapsed();
}

Stopwatch::clock::duration Stopwatch::elapsed() const
{
	return (running_ ? clock::now() : end_time_) - start_time_;
}

bool Stopwatch::running() const
{
	return running_;
}

Stopwatch::clock::time_point Stopwatch::start_time() const
{
	return start_time_;
}

Stopwatch::clock::time_point Stopwatch::end_time() const
{
	return end_time_;
}
