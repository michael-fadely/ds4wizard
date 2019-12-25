#pragma once

#include <algorithm>
#include <cmath>
#include <limits>

namespace gmath
{
	using namespace std;

	constexpr double pi           = M_PI;
	constexpr double two_pi       = 2.0 * pi;
	constexpr double pi_over_two  = M_PI_2;
	constexpr double pi_over_four = M_PI_4;
	constexpr double one_over_pi  = M_1_PI;
	constexpr double two_over_pi  = M_2_PI;

	constexpr float pi_f           = static_cast<float>(pi);
	constexpr float two_pi_f       = static_cast<float>(two_pi);
	constexpr float pi_over_two_f  = static_cast<float>(pi_over_two);
	constexpr float pi_over_four_f = static_cast<float>(pi_over_four);
	constexpr float one_over_pi_f  = static_cast<float>(one_over_pi);
	constexpr float two_over_pi_f  = static_cast<float>(two_over_pi);

	template <typename T>
	constexpr T epsilon()
	{
		return numeric_limits<T>::epsilon();
	}

	template <typename T>
	constexpr T wrap_e(T value, T low, T high)
	{
		if (value > high)
		{
			return low + (value - high);
		}

		if (value < low)
		{
			return high - (low - value);
		}

		return value;
	}

	template <typename T>
	constexpr T wrap_i(T value, T low, T high)
	{
		if (value >= high)
		{
			return low + (value - high);
		}

		if (value <= low)
		{
			return high - (low - value);
		}

		return value;
	}

	inline bool is_one(double value)
	{
		return abs(value - 1.0) < gmath::epsilon<double>();
	}

	inline bool is_one(float value)
	{
		return abs(value - 1.0f) < gmath::epsilon<float>();
	}

	inline bool is_zero(double value)
	{
		return abs(value) < gmath::epsilon<double>();
	}

	inline bool is_zero(float value)
	{
		return abs(value) < gmath::epsilon<float>();
	}

	inline bool near_equal(double a, double b)
	{
		return abs(a - b) < gmath::epsilon<double>();
	}

	inline bool near_equal(float a, float b)
	{
		return abs(a - b) < gmath::epsilon<float>();
	}

	template <typename T>
	T sign(T val)
	{
		return static_cast<T>(static_cast<int>((T(0) < val) - (val < T(0))));
	}

	template <typename T>
	T lerp(const T& a, const T& b, float factor)
	{
		return (1.0f - factor) * a + factor * b;
	}

	template <typename T>
	T lerp(const T& a, const T& b, double factor)
	{
		return (1.0 - factor) * a + factor * b;
	}

	/**
	 * \brief Performs smooth (cubic Hermite) interpolation between 0 and 1.
	 * \remarks See https://en.wikipedia.org/wiki/Smoothstep
	 * \param amount Value between 0 and 1 indicating interpolation amount.
	 */
	inline float smoothstep(float amount)
	{
		if (amount <= 0)
		{
			return 0;
		}

		return amount >= 1 ? 1 : amount * amount * (3 - 2 * amount);
	}

	/**
	 * \brief Performs a smooth(er) interpolation between 0 and 1 with 1st and 2nd order derivatives of zero at endpoints.
	 * \remarks See https://en.wikipedia.org/wiki/Smoothstep
	 * \param amount Value between 0 and 1 indicating interpolation amount.
	 */
	inline float smootherstep(float amount)
	{
		if (amount <= 0)
		{
			return 0;
		}

		return amount >= 1 ? 1 : amount * amount * amount * (amount * (amount * 6 - 15) + 10);
	}
}
