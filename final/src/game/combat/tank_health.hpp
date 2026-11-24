#pragma once
#include "core/object/component.hpp"

namespace tanks {
struct TankHealth final : Component {
	explicit TankHealth(float maximum) : maximum(maximum), current(maximum) {}

	void reset() { current = maximum; }
	bool takeDamage(float damage);

	float maximum;
	float current;
};
} // namespace tanks
