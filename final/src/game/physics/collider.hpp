#pragma once
#include "core/math.hpp"
#include "core/object/component.hpp"

namespace tanks {
struct BoxCollider final : Component {
	BoxCollider(Vec2 halfExtent, float boxHeight)
	    : halfSize(halfExtent), height(boxHeight) {}
	Vec2 halfSize;
	float height;
};

struct CircleCollider final : Component {
	explicit CircleCollider(float tankRadius) : radius(tankRadius) {}
	float radius;
};
} // namespace tanks
