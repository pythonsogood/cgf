#include "game/tank.hpp"
#include "game/ai/bot_input.hpp"
#include "game/input/player_input.hpp"
#include "game/world/world.hpp"

namespace tanks {
Tank::Tank(TankId index, TeamId team, Controller controller, const Rules &rules)
    : id(index), control(team, controller), collider(rules.tankRadius),
      motion(rules.tankSpeed, collider), health(rules.maxHealth) {
	attachComponent(control);
	attachComponent(collider);
	attachComponent(motion);
	attachComponent(weapon);
	attachComponent(health);
	attachComponent(mesh_);

	if (controller == Controller::Human)
		addComponent<PlayerInput>(*this);
	else
		addComponent<BotInput>(*this);
}

void Tank::apply(const Command &command, float dt, World &world) {
	weapon.updateAim(command, dt, world.config().rules);
	motion.move(command.move, dt, world);
	weapon.tryFire(command, *this, world);
}
} // namespace tanks
