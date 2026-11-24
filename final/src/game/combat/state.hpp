#pragma once
#include "game/combat/projectile.hpp"
#include "game/combat/effect.hpp"
#include <array>
#include <cstdint>

namespace tanks::detail {
struct CombatState {
	std::array<Projectile, maxProjectiles> projectiles{};
	std::array<Effect, maxEffects> effects{};
	std::uint64_t shotsFired = 0;
	std::uint64_t damageEvents = 0;
};
} // namespace tanks::detail
