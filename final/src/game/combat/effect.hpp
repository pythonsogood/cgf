#pragma once
#include "core/color.hpp"
#include "core/object/game_object.hpp"
#include "game/render/models.hpp"
#include <array>

namespace tanks {
struct Spark {
	Vec2 velocity;
	float lift, lifetime, size;
};

struct Blast final : Component {
	Color color;
	float age = 0.0f;
	float duration = 0.5f;
	std::array<Spark, 20> sparks{};
};

class Effect final : public GameObject {
	public:
	Effect();
	Blast effect;

	private:
	BlastRenderer mesh_;
};
} // namespace tanks
