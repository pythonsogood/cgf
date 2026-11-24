#include "core/model_matrix.hpp"
#include <cmath>

namespace tanks {
M4 identity() {
	M4 r;

	r.a[0] = r.a[5] = r.a[10] = r.a[15] = 1;

	return r;
}

M4 multiply(const M4 &a, const M4 &b) {
	M4 r;

	for (int c = 0; c < 4; ++c)
		for (int row = 0; row < 4; ++row)
			for (int k = 0; k < 4; ++k)
				r.a[c * 4 + row] += a.a[k * 4 + row] * b.a[c * 4 + k];

	return r;
}

M4 translate(float x, float y, float z) {
	M4 r = identity();

	r.a[12] = x;
	r.a[13] = y;
	r.a[14] = z;

	return r;
}

M4 scale(float x, float y, float z) {
	M4 r = identity();

	r.a[0] = x;
	r.a[5] = y;
	r.a[10] = z;

	return r;
}

M4 rotateX(float a) {
	M4 r = identity();
	float c = std::cos(a), s = std::sin(a);

	r.a[5] = c;
	r.a[6] = s;
	r.a[9] = -s;
	r.a[10] = c;

	return r;
}

M4 rotateY(float a) {
	M4 r = identity();
	float c = std::cos(a), s = std::sin(a);

	r.a[0] = c;
	r.a[2] = -s;
	r.a[8] = s;
	r.a[10] = c;

	return r;
}

M4 rotateZ(float a) {
	M4 r = identity();
	float c = std::cos(a), s = std::sin(a);

	r.a[0] = c;
	r.a[1] = s;
	r.a[4] = -s;
	r.a[5] = c;

	return r;
}
} // namespace tanks
