#pragma once

template <typename T, ptrdiff_t n>
struct circular_buffer
{
private:
	std::array<T, n> points;

	ptrdiff_t l = n - 1;
	ptrdiff_t i = 0;

public:
	const ptrdiff_t count = n;

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

	const T& operator[](ptrdiff_t index) const
	{
		return points[(index + n + i) % n];
	}
};
