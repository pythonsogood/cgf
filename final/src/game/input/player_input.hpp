#pragma once
#include "game/input/control_behaviour.hpp"

namespace tanks {
class Tank;
class PlayerInput final : public ControlBehaviour {
	public:
	explicit PlayerInput(Tank &tank) : tank_(tank) {}
	void updateControls(const PlayerInputs &inputs, float dt,
	                    World &world) override;

	private:
	Tank &tank_;
};
} // namespace tanks
