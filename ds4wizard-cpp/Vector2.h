#pragma once

class Vector2
{
public:
	static const Vector2 zero;
	static const Vector2 one;

	union
	{
	#pragma pack(push, 1)
		struct
		{
			float x, y;
		};

		float array[2];
	#pragma pack(pop)
	};

	Vector2() = default;
	Vector2(float x, float y);

	explicit Vector2(float f);

	float operator[](size_t i) const;

	Vector2 operator+() const;
	Vector2 operator-() const;

	Vector2 operator+(const Vector2& rhs) const;
	Vector2 operator-(const Vector2& rhs) const;
	Vector2 operator*(const Vector2& rhs) const;
	Vector2 operator/(const Vector2& rhs) const;

	void operator+=(const Vector2& rhs);
	void operator-=(const Vector2& rhs);
	void operator/=(const Vector2& rhs);
	void operator*=(const Vector2& rhs);

	Vector2 operator+(float rhs) const;
	Vector2 operator-(float rhs) const;
	Vector2 operator*(float rhs) const;
	Vector2 operator/(float rhs) const;

	void operator+=(float rhs);
	void operator-=(float rhs);
	void operator/=(float rhs);
	void operator*=(float rhs);

	explicit operator float*();
	explicit operator const float*() const;

	float length_squared() const;
	float length() const;
	void normalize();
	Vector2 normalized() const;
	bool operator==(const Vector2& rhs) const;
	bool operator!=(const Vector2& rhs) const;
	bool is_normalized() const;

	float dot(const Vector2& rhs) const;
	bool near_equal(const Vector2& rhs) const;
};

Vector2 operator*(float lhs, const Vector2& rhs);
