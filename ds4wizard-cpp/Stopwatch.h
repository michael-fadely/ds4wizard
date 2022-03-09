#pragma once

#include <chrono>

class Stopwatch
{
public:
	using Clock     = std::chrono::high_resolution_clock;
	using TimePoint = Clock::time_point;
	using Duration  = Clock::duration;

protected:
	bool running_ = false;
	TimePoint start_time_;
	TimePoint end_time_;

public:
	Stopwatch() = default;
	explicit Stopwatch(bool start_now);

	void start();
	Duration stop();
	[[nodiscard]] Duration elapsed() const;
	[[nodiscard]] bool running() const;
	[[nodiscard]] TimePoint start_time() const;
	[[nodiscard]] TimePoint end_time() const;
};
