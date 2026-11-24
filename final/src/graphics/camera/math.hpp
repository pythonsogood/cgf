#pragma once
#include "core/math.hpp"
#include "core/model_matrix.hpp"

namespace tanks::graphics {
struct V3 {
	float x, y, z;
};

V3 sub(V3 a, V3 b);

V3 mul(V3 value, float scale);

float dot3(V3 a, V3 b);

V3 cross(V3 a, V3 b);

V3 norm(V3 value);

M4 perspective(float fov, float aspect, float nearPlane, float farPlane);

M4 ortho(float left, float right, float bottom, float top, float nearPlane,
         float farPlane);

M4 lookAt(V3 eye, V3 target);
} // namespace tanks::graphics
