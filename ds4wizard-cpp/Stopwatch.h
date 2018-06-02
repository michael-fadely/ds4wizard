#pragma once

#include <chrono>

class Stopwatch
{
public:
	using clock = std::chrono::high_resolution_clock;

protected:
	bool running_ = false;
	clock::time_point start_time_;
	clock::time_point end_time_;

public:
	Stopwatch() = default;
	explicit Stopwatch(bool start_now);

	void start();
	clock::duration stop();
	clock::duration elapsed() const;
	bool running() const;
	clock::time_point start_time() const;
	clock::time_point end_time() const;
};
