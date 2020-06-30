#pragma once

class Vector3
{
public:
	static const Vector3 zero;
	static const Vector3 one;

	union
	{
	#pragma pack(push, 1)
		struct
		{
			float x, y, z;
		};

		float array[3];
	#pragma pack(pop)
	};

	Vector3() = default;
	Vector3(float x, float y, float z);

	explicit Vector3(float f);

	float operator[](size_t i) const;

	Vector3 operator+() const;
	Vector3 operator-() const;

	Vector3 operator+(const Vector3& rhs) const;
	Vector3 operator-(const Vector3& rhs) const;
	Vector3 operator*(const Vector3& rhs) const;
	Vector3 operator/(const Vector3& rhs) const;

	void operator+=(const Vector3& rhs);
	void operator-=(const Vector3& rhs);
	void operator/=(const Vector3& rhs);
	void operator*=(const Vector3& rhs);

	Vector3 operator+(float rhs) const;
	Vector3 operator-(float rhs) const;
	Vector3 operator*(float rhs) const;
	Vector3 operator/(float rhs) const;

	void operator+=(float rhs);
	void operator-=(float rhs);
	void operator/=(float rhs);
	void operator*=(float rhs);

	explicit operator float*();
	explicit operator const float*() const;

	[[nodiscard]] float lengthSquared() const;
	[[nodiscard]] float length() const;
	void normalize();
	[[nodiscard]] Vector3 normalized() const;
	bool operator==(const Vector3& rhs) const;
	bool operator!=(const Vector3& rhs) const;
	[[nodiscard]] bool isNormalized() const;

	[[nodiscard]] float dot(const Vector3& rhs) const;
	[[nodiscard]] bool nearEqual(const Vector3& rhs) const;

	static Vector3 lerp(const Vector3& start, const Vector3& end, float amount);
	static Vector3 clamp(const Vector3& v, float lower, float upper);
};

Vector3 operator*(float lhs, const Vector3& rhs);
