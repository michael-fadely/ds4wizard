#pragma once

template <typename T, size_t n>
struct circular_buffer
{
	const size_t count = n;
	std::array<T, n> points;

	size_t l = n - 1;
	size_t i = 0;

	void insert(const T& value)
	{
		l = i;
		points[i++] = value;
		i %= n;
	}

	void fill(const T& value)
	{
		for (auto& p : points)
		{
			p = value;
		}
	}

	[[nodiscard]] T newest() const
	{
		return points[l];
	}

	[[nodiscard]] T oldest() const
	{
		return points[i];
	}

	const T& operator[](size_t index) const
	{
		return points[(index + i) % n];
	}
};
