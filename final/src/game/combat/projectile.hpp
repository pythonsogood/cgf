#pragma once
#include "core/object/game_object.hpp"
#include "game/identity.hpp"
#include "game/render/models.hpp"

namespace tanks {
inline constexpr float barrelHeight = 1.16f;
struct ProjectileFlight final : Component {
	Vec2 velocity;
	TeamId team = noTeam;
	TankId owner = noTank;
	float life = 0.0f;
};

class Projectile final : public GameObject {
	public:
	Projectile();
	ProjectileFlight flight;

	private:
	ShellRenderer mesh_;
};
} // namespace tanks
