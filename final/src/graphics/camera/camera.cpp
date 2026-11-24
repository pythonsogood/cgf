#include "graphics/camera/camera.hpp"
#include <algorithm>
#include <cmath>
namespace tanks::graphics {
namespace {
constexpr float fov = .78f;
}

void Camera::resize(int w, int h) {
	width = std::max(w, 1);
	height = std::max(h, 1);
}

void Camera::orbit(float dx, float dy) {
	yaw -= dx * .006f;
	elevation = std::clamp(elevation + dy * .006f, .22f, 1.38f);
}

void Camera::zoom(float steps) {
	distance = std::clamp(distance * std::pow(.90f, steps), 30.0f, 105.0f);
}

void Camera::reset() {
	yaw = pi / 4;
	elevation = .615479709f; // atan(1 / sqrt(2))
	distance = 54.0f;
}

M4 Camera::viewProjection(bool orthographic) {
	float ce = std::cos(elevation), se = std::sin(elevation);
	eye = {distance * ce * std::sin(yaw), distance * se,
	       distance * ce * std::cos(yaw)};

	M4 view = lookAt(eye, {0, 0, 0});
	float aspect = static_cast<float>(std::max(width, 1)) / std::max(height, 1);
	float span = distance * std::tan(fov * .5f);

	return multiply(
	    orthographic ? ortho(-span * aspect, span * aspect, -span, span, .1f, 180)
					             : perspective(fov, aspect, .1f, 180),
	    view);
}

Vec2 Camera::screenRight() const { return {std::cos(yaw), -std::sin(yaw)}; }
} // namespace tanks::graphics
