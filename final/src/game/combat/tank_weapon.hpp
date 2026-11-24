#pragma once
#include "core/object/component.hpp"
#include "game/command.hpp"
#include "game/match/config.hpp"

namespace tanks {
class Tank;
class World;

struct TankWeapon final : Component {
	float turretAngle = 0.0f;
	float cooldown = 0.0f;
	float muzzleFlash = 0.0f;

	void updateAim(const Command &command, float dt, const Rules &rules);

	bool tryFire(const Command &command, const Tank &owner, World &world);

	void reset(float yaw);
};
} // namespace tanks
