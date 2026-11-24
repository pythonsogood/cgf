#pragma once
#include "game/combat/tank_health.hpp"
#include "game/combat/tank_weapon.hpp"
#include "core/object/game_object.hpp"
#include "game/command.hpp"
#include "game/identity.hpp"
#include "game/match/config.hpp"
#include "game/physics/movement.hpp"
#include "game/render/mesh_renderer.hpp"
#include "game/render/models.hpp"

namespace tanks {
struct TankControl final : Component {
	TankControl(TeamId side, Controller input) : team(side), controller(input) {}
	const TeamId team;
	const Controller controller;
};

class Tank final : public GameObject {
	public:
	Tank(TankId index, TeamId team, Controller controller, const Rules &rules);
	void apply(const Command &command, float dt, World &world);

	const TankId id;
	TankControl control;
	CircleCollider collider;
	TankMotion motion;
	TankWeapon weapon;
	TankHealth health;

	private:
	TankRenderer mesh_;
};
} // namespace tanks
