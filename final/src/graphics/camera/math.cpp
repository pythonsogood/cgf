#include "graphics/camera/math.hpp"
#include <cmath>
namespace tanks::graphics {
V3 sub(V3 a, V3 b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }

V3 mul(V3 a, float s) { return {a.x * s, a.y * s, a.z * s}; }

float dot3(V3 a, V3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

V3 cross(V3 a, V3 b) {
	return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

V3 norm(V3 a) {
	float n = std::hypot(a.x, a.y, a.z);
	return n > 1e-8f ? mul(a, 1.0f / n) : V3{0, 1, 0};
}

M4 perspective(float fov, float aspect, float n, float f) {
	M4 r;
	float v = 1 / std::tan(fov * .5f);
	r.a[0] = v / aspect;
	r.a[5] = v;
	r.a[10] = -(f + n) / (f - n);
	r.a[11] = -1;
	r.a[14] = -(2 * f * n) / (f - n);
	return r;
}

M4 ortho(float l, float r, float b, float t, float n, float f) {
	M4 m = identity();
	m.a[0] = 2 / (r - l);
	m.a[5] = 2 / (t - b);
	m.a[10] = -2 / (f - n);
	m.a[12] = -(r + l) / (r - l);
	m.a[13] = -(t + b) / (t - b);
	m.a[14] = -(f + n) / (f - n);
	return m;
}

M4 lookAt(V3 eye, V3 target) {
	V3 f = norm(sub(target, eye)), s = norm(cross(f, {0, 1, 0})), u = cross(s, f);
	M4 r = identity();
	r.a[0] = s.x;
	r.a[4] = s.y;
	r.a[8] = s.z;
	r.a[1] = u.x;
	r.a[5] = u.y;
	r.a[9] = u.z;
	r.a[2] = -f.x;
	r.a[6] = -f.y;
	r.a[10] = -f.z;
	r.a[12] = -dot3(s, eye);
	r.a[13] = -dot3(u, eye);
	r.a[14] = dot3(f, eye);
	return r;
}
} // namespace tanks::graphics
