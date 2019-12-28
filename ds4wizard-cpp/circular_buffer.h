#pragma once

template <typename T, size_t n>
struct circular_buffer
{
	std::array<T, n> points;

	size_t l = n - 1;
	size_t i = 0;

	void insert(T value)
	{
		l = i;
		points[i++] = value;
		i %= n;
	}

	void fill(T value)
	{
		for (auto& p : points)
		{
			p = value;
		}
	}

	[[nodiscard]] T newest() const
	{
		return points[i - 1];
	}

	[[nodiscard]] T oldest() const
	{
		return points[i];
	}
};
