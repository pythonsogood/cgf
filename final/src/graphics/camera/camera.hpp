#pragma once
#include "graphics/camera/math.hpp"

namespace tanks::graphics {
struct Camera {
	int width = 1280, height = 720;

	Camera() { reset(); }

	float yaw, elevation, distance;

	V3 eye{};

	void resize(int width, int height);

	void orbit(float deltaX, float deltaY);

	void zoom(float steps);

	void reset();

	M4 viewProjection(bool orthographic);

	Vec2 screenRight() const;
};
} // namespace tanks::graphics
