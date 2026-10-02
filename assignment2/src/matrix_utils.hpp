#pragma once

#include <cmath>

namespace matrix_utils {

using Matrix = float[16];

inline void identity(Matrix &result) {
	for (int i = 0; i < 16; ++i) {
		result[i] = i % 5 == 0 ? 1.0F : 0.0F;
	}
}

inline void multiply(Matrix &result, const Matrix &a, const Matrix &b) {
	Matrix product{};

	for (int row = 0; row < 4; ++row) {
		for (int column = 0; column < 4; ++column) {
			for (int k = 0; k < 4; ++k) {
				product[row * 4 + column] += a[row * 4 + k] * b[k * 4 + column];
			}
		}
	}

	for (int i = 0; i < 16; ++i) {
		result[i] = product[i];
	}
}

inline void scale(Matrix &result, float x, float y, float z = 1.0F) {
	identity(result);

	result[0] = x;
	result[5] = y;
	result[10] = z;
}

inline void translate(Matrix &result, float x, float y, float z = 0.0F) {
	identity(result);

	result[3] = x;
	result[7] = y;
	result[11] = z;
}

inline void rotate(Matrix &result, float degrees, int axis) {
	float radians = degrees * M_PI / 180.0F;
	float c = std::cos(radians);
	float s = std::sin(radians);

	identity(result);

	if (axis == 0) {
		result[5] = c;
		result[6] = -s;
		result[9] = s;
		result[10] = c;
	} else if (axis == 1) {
		result[0] = c;
		result[2] = s;
		result[8] = -s;
		result[10] = c;
	} else {
		result[0] = c;
		result[1] = -s;
		result[4] = s;
		result[5] = c;
	}
}

} // namespace matrix_utils
