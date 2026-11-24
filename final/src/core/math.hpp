#pragma once
#include <numbers>

namespace tanks {
inline constexpr float pi = std::numbers::pi_v<float>;

struct Vec2 {
	float x = 0.0f;
	float z = 0.0f;
	Vec2 operator+(Vec2 b) const { return {x + b.x, z + b.z}; }
	Vec2 operator-(Vec2 b) const { return {x - b.x, z - b.z}; }
	Vec2 operator*(float s) const { return {x * s, z * s}; }
	Vec2 operator/(float s) const { return {x / s, z / s}; }
	Vec2 &operator+=(Vec2 b) {
		x += b.x;
		z += b.z;
		return *this;
	}
};

float dot(Vec2 a, Vec2 b);
float length(Vec2 v);
Vec2 normalized(Vec2 v);
float angleOf(Vec2 v);
Vec2 direction(float angle);
float wrapAngle(float angle);
} // namespace tanks
