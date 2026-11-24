#pragma once
#include "core/object/component.hpp"
#include "game/command.hpp"

namespace tanks {
class World;
class GameObject;
void updateControls(GameObject &object, const PlayerInputs &inputs, float dt,
                    World &world);

class ControlBehaviour : public Component {
	public:
	virtual void updateControls(const PlayerInputs &inputs, float dt,
	                            World &world) = 0;
	virtual void resetControls() {}
};
} // namespace tanks
