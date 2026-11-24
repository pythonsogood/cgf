#pragma once
#include "core/object/game_object.hpp"
#include "game/physics/collider.hpp"
#include "game/render/models.hpp"
#include <array>
#include <cstddef>
#include <deque>

namespace tanks {
inline constexpr std::size_t arenaSides = 7;
using ArenaOutline = std::array<Vec2, arenaSides>;
const ArenaOutline &unitArenaOutline();

class Obstacle final : public GameObject {
	public:
	Obstacle(Vec2 center, Vec2 halfSize, float height = 1.8f, float yaw = 0.0f);
	BoxCollider collider;

	private:
	CoverRenderer mesh_;
};

class Arena final : public GameObject {
	public:
	explicit Arena(float radius);

	const ArenaOutline &outline() const { return outline_; }

	const std::deque<Obstacle> &obstacles() const { return obstacles_; }

	bool contains(Vec2 point, float margin = 0.0f) const;

	Vec2 confine(Vec2 point, float margin = 0.0f) const;

	bool traceBoundary(Vec2 start, Vec2 end, float margin, float &fraction) const;

	private:
	ArenaRenderer mesh_;
	float inradius_;
	ArenaOutline outline_;
	std::array<Vec2, arenaSides> inwardNormals_;
	std::deque<Obstacle> obstacles_;
};
} // namespace tanks
