#include "game/world/arena.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace tanks {
namespace {
ArenaOutline makeUnitArenaOutline() {
	ArenaOutline vertices{};

	for (std::size_t i = 0; i < arenaSides; ++i) {
		const float angle = -pi * 0.5f + 2.0f * pi * float(i) / float(arenaSides);
		vertices[i] = {std::cos(angle), std::sin(angle)};
	}

	return vertices;
}
} // namespace

const ArenaOutline &unitArenaOutline() {
	static const ArenaOutline outline = makeUnitArenaOutline();
	return outline;
}

Obstacle::Obstacle(Vec2 position, Vec2 extent, float h, float yaw)
    : collider(extent, h) {
	transform().position = position;
	transform().setYaw(yaw);

	attachComponent(collider);
	attachComponent(mesh_);
}

Arena::Arena(float radius)
    : inradius_(radius * std::cos(pi / float(arenaSides))) {
	if (!std::isfinite(radius) || radius <= 0.0f)
		throw std::invalid_argument("arena radius must be finite and positive");

	attachComponent(mesh_);

	const auto &unit = unitArenaOutline();

	for (std::size_t i = 0; i < arenaSides; ++i) {
		outline_[i] = unit[i] * radius;
		const Vec2 edge = unit[(i + 1) % arenaSides] - unit[i];
		inwardNormals_[i] = normalized({-edge.z, edge.x});
	}

	// Reconstructed from Motor Murder mini-game from Pummel Party
	const float s = radius / 19.0f;

	obstacles_.emplace_back(Vec2{-8.8f, -8.0f} * s, Vec2{1.5f, 1.65f} * s, 2.2f);
	obstacles_.emplace_back(Vec2{0.0f, -11.4f} * s, Vec2{2.9f, 2.4f} * s, 3.1f);
	obstacles_.emplace_back(Vec2{8.8f, -8.0f} * s, Vec2{1.5f, 1.65f} * s, 2.2f);
	obstacles_.emplace_back(Vec2{-11.3f, 0.0f} * s, Vec2{1.7f, 2.0f} * s, 2.3f);
	obstacles_.emplace_back(Vec2{11.3f, 0.0f} * s, Vec2{2.0f, 2.0f} * s, 2.3f);
	obstacles_.emplace_back(Vec2{-1.7f, -1.2f} * s, Vec2{3.0f, 0.65f} * s, 1.6f,
	                        pi * 0.23f);
	obstacles_.emplace_back(Vec2{1.3f, -0.1f} * s, Vec2{1.8f, 0.65f} * s, 1.3f,
	                        -pi * 0.20f);

	for (int row = 0; row < 2; ++row)
		for (int column = 0; column < 4; ++column)
			obstacles_.emplace_back(Vec2{-3.0f + 2.0f * column, 8.15f + 2.7f * row} * s,
			                        Vec2{0.95f, 1.15f} * s, 1.8f);
}

bool Arena::contains(Vec2 point, float margin) const {
	for (std::size_t i = 0; i < arenaSides; ++i)
		if (dot(point - outline_[i], inwardNormals_[i]) < margin - 0.00001f)
			return false;

	return true;
}

Vec2 Arena::confine(Vec2 point, float margin) const {
	if (contains(point, margin))
		return point;

	const float inset = std::max(0.0f, 1.0f - margin / inradius_);
	Vec2 closest{};
	float nearest = std::numeric_limits<float>::infinity();

	for (std::size_t i = 0; i < arenaSides; ++i) {
		const Vec2 a = outline_[i] * inset;
		const Vec2 edge = (outline_[(i + 1) % arenaSides] - outline_[i]) * inset;
		const float squaredLength = dot(edge, edge);
		const float t =
		    squaredLength > 0.0f
		        ? std::clamp(dot(point - a, edge) / squaredLength, 0.0f, 1.0f)
		        : 0.0f;
		const Vec2 candidate = a + edge * t;
		const float distance = dot(point - candidate, point - candidate);

		if (distance < nearest) {
			nearest = distance;
			closest = candidate;
		}
	}

	return closest;
}

bool Arena::traceBoundary(Vec2 start, Vec2 end, float margin,
                          float &fraction) const {
	fraction = 1.0f;
	bool hit = false;

	for (std::size_t i = 0; i < arenaSides; ++i) {
		const float from = dot(start - outline_[i], inwardNormals_[i]) - margin;
		const float to = dot(end - outline_[i], inwardNormals_[i]) - margin;

		if (from < -0.00001f) {
			fraction = 0.0f;
			return true;
		}

		if (to >= 0.0f)
			continue;

		const float entry = std::clamp(from / (from - to), 0.0f, 1.0f);
		fraction = std::min(fraction, entry);
		hit = true;
	}

	return hit;
}
} // namespace tanks
