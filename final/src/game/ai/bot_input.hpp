#pragma once
#include "game/input/control_behaviour.hpp"
#include "game/tank.hpp"
#include <array>
#include <cstddef>

namespace tanks {
class BotInput final : public ControlBehaviour {
	public:
	explicit BotInput(Tank &tank) : tank_(tank) {}
	void updateControls(const PlayerInputs &inputs, float dt,
	                    World &world) override;
	void resetControls() override;

	private:
	void planPath(Vec2 goal, World &world);

	struct Navigation {
		TankId target = noTank;
		float replan = 0.0f;
		float fireDelay = 0.0f;
		std::array<Vec2, 1600> path{};
		std::size_t pathSize = 0;
		std::size_t waypoint = 0;
	};

	Tank &tank_;
	Navigation bot_;
};
} // namespace tanks
