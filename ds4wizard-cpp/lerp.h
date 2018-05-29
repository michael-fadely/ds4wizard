#pragma once

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
