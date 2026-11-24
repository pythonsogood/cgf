#pragma once

namespace tanks {
struct M4 {
	float a[16]{};
};

M4 identity();

M4 multiply(const M4 &a, const M4 &b);

inline M4 operator*(const M4 &a, const M4 &b) { return multiply(a, b); }

M4 translate(float x, float y, float z);

M4 scale(float x, float y, float z);

M4 rotateX(float angle);

M4 rotateY(float angle);

M4 rotateZ(float angle);
} // namespace tanks
