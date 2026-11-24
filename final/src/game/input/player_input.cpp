#include "game/input/player_input.hpp"
#include "game/tank.hpp"

namespace tanks {
void PlayerInput::updateControls(const PlayerInputs &inputs, float dt,
                                 World &world) {
	const PlayerActions &input = inputs.player;
	const Keyboard &keys = inputs.keyboard;
	Command command;

	command.move =
	    input.move +
	    inputs.screenRight * float(keys.isDown('d') - keys.isDown('a')) +
	    inputs.screenUp * float(keys.isDown('w') - keys.isDown('s'));
	command.fire = input.fire || inputs.primaryDown || keys.isDown(Key::Space);
	command.turn = float(keys.isDown('z')) - float(keys.isDown('x'));

	if (input.hasAim) {
		command.hasAim = true;
		command.aim = input.aim;
	}

	tank_.apply(command, dt, world);
}
} // namespace tanks
