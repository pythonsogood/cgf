#include "core/math.hpp"
#include <cmath>

namespace tanks {
float dot(Vec2 a, Vec2 b) { return a.x * b.x + a.z * b.z; }

float length(Vec2 v) { return std::hypot(v.x, v.z); }

Vec2 normalized(Vec2 v) {
	const float l = length(v);
	return l > 1.0e-6f ? v / l : Vec2{};
}

float angleOf(Vec2 v) { return std::atan2(v.x, v.z); }

Vec2 direction(float a) { return {std::sin(a), std::cos(a)}; }

float wrapAngle(float a) {
	if (!std::isfinite(a))
		return 0.0f;

	if (std::abs(a) <= pi)
		return a;

	const float turn = 2.0f * pi;
	float wrapped = std::fmod(a, turn);

	if (wrapped > pi)
		wrapped -= turn;
	else if (wrapped < -pi)
		wrapped += turn;

	return wrapped;
}
} // namespace tanks
