#include "pch.h"
#include "Vector3.h"
#include "gmath.h"

const Vector3 Vector3::zero = { 0.0f, 0.0f, 0.0f };
const Vector3 Vector3::one  = { 1.0f, 1.0f, 1.0f };

Vector3::Vector3(float f)  // NOLINT(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
	: x(f),
	  y(f),
	  z(f)
{
}

Vector3::Vector3(float x, float y, float z)  // NOLINT(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
	: x(x),
	  y(y),
	  z(z)
{
}

float Vector3::operator[](size_t i) const
{
	if (i >= 3)
	{
		throw;
	}

	return array[i];
}

Vector3 Vector3::operator+() const
{
	const Vector3 result = { +x, +y, +z };
	return result;
}

Vector3 Vector3::operator-() const
{
	const Vector3 result = { -x, -y, -z };
	return result;
}

Vector3 Vector3::operator+(const Vector3& rhs) const
{
	const Vector3 result = {
		x + rhs.x,
		y + rhs.y,
		z + rhs.z
	};

	return result;
}

Vector3 Vector3::operator-(const Vector3& rhs) const
{
	const Vector3 result = {
		x - rhs.x,
		y - rhs.y,
		z - rhs.z
	};

	return result;
}

Vector3 Vector3::operator*(const Vector3& rhs) const
{
	const Vector3 result = {
		x * rhs.x,
		y * rhs.y,
		z * rhs.z
	};

	return result;
}

Vector3 Vector3::operator/(const Vector3& rhs) const
{
	const Vector3 result = {
		x / rhs.x,
		y / rhs.y,
		z / rhs.z
	};

	return result;
}

void Vector3::operator+=(const Vector3& rhs)
{
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
}

void Vector3::operator-=(const Vector3& rhs)
{
	x -= rhs.x;
	y -= rhs.y;
	z -= rhs.z;
}

void Vector3::operator/=(const Vector3& rhs)
{
	x /= rhs.x;
	y /= rhs.y;
	z /= rhs.z;
}

void Vector3::operator*=(const Vector3& rhs)
{
	x *= rhs.x;
	y *= rhs.y;
	z *= rhs.z;
}

Vector3 Vector3::operator+(const float rhs) const
{
	const Vector3 result = {
		x + rhs,
		y + rhs,
		z + rhs
	};

	return result;
}

Vector3 Vector3::operator-(const float rhs) const
{
	const Vector3 result = {
		x - rhs,
		y - rhs,
		z - rhs
	};

	return result;
}

Vector3 Vector3::operator*(const float rhs) const
{
	const Vector3 result = {
		x * rhs,
		y * rhs,
		z * rhs
	};

	return result;
}

Vector3 Vector3::operator/(const float rhs) const
{
	const Vector3 result = {
		x / rhs,
		y / rhs,
		z / rhs
	};

	return result;
}

void Vector3::operator+=(const float rhs)
{
	x += rhs;
	y += rhs;
	z += rhs;
}

void Vector3::operator-=(const float rhs)
{
	x -= rhs;
	y -= rhs;
	z -= rhs;
}

void Vector3::operator/=(const float rhs)
{
	x /= rhs;
	y /= rhs;
	z /= rhs;
}

void Vector3::operator*=(const float rhs)
{
	x *= rhs;
	y *= rhs;
	z *= rhs;
}

Vector3::operator float*()
{
	return array;
}

Vector3::operator const float*() const
{
	return array;
}

float Vector3::lengthSquared() const
{
	return x * x + y * y + z * z;
}

float Vector3::length() const
{
	return sqrt(lengthSquared());
}

void Vector3::normalize()
{
	const auto l = length();

	if (l == 0.0f)
	{
		x = y = z = 0.0f;
	}
	else
	{
		x = x / l;
		y = y / l;
		z = z / l;
	}
}

Vector3 Vector3::normalized() const
{
	Vector3 result = *this;
	result.normalize();
	return result;
}

bool Vector3::operator==(const Vector3& rhs) const
{
	return gmath::near_equal(x, rhs.x) &&
	       gmath::near_equal(y, rhs.y) &&
	       gmath::near_equal(z, rhs.z);
}

bool Vector3::operator!=(const Vector3& rhs) const
{
	return !(*this == rhs);
}

bool Vector3::isNormalized() const
{
	return gmath::is_one(lengthSquared());
}

float Vector3::dot(const Vector3& rhs) const
{
	return (x * rhs.x) + (y * rhs.y) + (z * rhs.z);
}

bool Vector3::nearEqual(const Vector3& rhs) const
{
	return gmath::near_equal(x, rhs.x) &&
	       gmath::near_equal(y, rhs.y) &&
	       gmath::near_equal(z, rhs.z);
}

Vector3 Vector3::lerp(const Vector3& start, const Vector3& end, float amount)
{
	return
	{
		gmath::lerp(start.x, end.x, amount),
		gmath::lerp(start.y, end.y, amount),
		gmath::lerp(start.z, end.z, amount)
	};
}

Vector3 Vector3::clamp(const Vector3& v, float lower, float upper)
{
	return
	{
		std::clamp(v.x, lower, upper),
		std::clamp(v.y, lower, upper),
		std::clamp(v.z, lower, upper)
	};
}

Vector3 operator*(float lhs, const Vector3& rhs)
{
	return rhs * lhs;
}
