#include "game/combat/tank_health.hpp"
#include <algorithm>

namespace tanks {
bool TankHealth::takeDamage(float damage) {
	if (!(damage > 0.0f) || current <= 0.0f)
		return false;

	current = std::max(0.0f, current - damage);

	return current == 0.0f;
}
} // namespace tanks
