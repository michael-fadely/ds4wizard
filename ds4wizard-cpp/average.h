#pragma once

#include <vector>

#include "Stopwatch.h"

template <typename T>
class timed_average
{
private:
	std::vector<T> points;
	Stopwatch stopwatch;
	Stopwatch::Duration target_duration;
	T last_average {};
	bool dirty = false;

public:
	timed_average(Stopwatch::Duration dur)
		: target_duration(dur)
	{
	}

	void push(const T& value)
	{
		if (!stopwatch.running())
		{
			stopwatch.start();
		}

		dirty = true;
		points.push_back(value);

		// if we've met our target, cache the result now and clear the buffer
		if (stopwatch.elapsed() >= target_duration)
		{
			this->value();
		}
	}

	T value()
	{
		if (!dirty || stopwatch.elapsed() < target_duration)
		{
			return last_average;
		}

		if (dirty)
		{
			T temp {};

			for (const auto& p : points)
			{
				temp += p / points.size();
			}

			last_average = temp;
			dirty = false;
		}

		if (stopwatch.elapsed() >= target_duration)
		{
			points.clear();
			points.push_back(last_average);
			stopwatch.start();
		}

		return last_average;
	}
};

template <typename T, size_t POINT_COUNT>
class average
{
private:
	bool dirty = false;
	size_t point_i = 0;
	size_t point_count = 0;
	std::array<T, POINT_COUNT> points;
	T last_average {};

public:
	void push(const T& value)
	{
		dirty = true;
		points[point_i] = value;
		++point_i %= points.size();
		
		if (point_count < points.size())
		{
			++point_count;
		}
	}

	T value()
	{
		if (!dirty || point_count < points.size())
		{
			return last_average;
		}

		T temp {};

		for (const auto& p : points)
		{
			temp += p / static_cast<T>(points.size());
		}

		last_average = temp;
		return temp;
	}
};
