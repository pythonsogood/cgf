#include "game/combat/tank_weapon.hpp"
#include "game/tank.hpp"
#include "game/world/world.hpp"
#include "game/physics/collision.hpp"
#include <algorithm>
#include <cmath>

namespace tanks {
void TankWeapon::updateAim(const Command &command, float dt,
                           const Rules &rules) {
	cooldown = std::max(0.0f, cooldown - dt);
	muzzleFlash = std::max(0.0f, muzzleFlash - dt);

	if (command.hasAim && detail::finite(command.aim)) {
		const float target = angleOf(command.aim - owner().transform().position);
		const float turn = wrapAngle(target - turretAngle);
		const float maxTurn = rules.turretTurnSpeed * dt;

		turretAngle = wrapAngle(turretAngle + std::clamp(turn, -maxTurn, maxTurn));
	} else if (std::isfinite(command.turn)) {
		const float maxTurn = rules.turretTurnSpeed * dt;

		turretAngle =
		    wrapAngle(turretAngle + std::clamp(command.turn, -1.0f, 1.0f) * maxTurn);
	}
}

bool TankWeapon::tryFire(const Command &command, const Tank &owner,
                         World &world) {
	if (!command.fire || !owner.active() || cooldown > 0.0f)
		return false;

	if (command.hasAim && detail::finite(command.aim) &&
	    std::abs(wrapAngle(angleOf(command.aim - owner.transform().position) -
	                       turretAngle)) >= 0.14f)
		return false;

	if (!world.spawnShell(owner))
		return false;

	cooldown = world.config().rules.fireCooldown;
	muzzleFlash = 0.08f;

	return true;
}

void TankWeapon::reset(float yaw) {
	turretAngle = yaw;
	cooldown = 0.0f;
	muzzleFlash = 0.0f;
}
} // namespace tanks
