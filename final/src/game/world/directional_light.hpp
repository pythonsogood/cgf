#pragma once
#include "core/object/game_object.hpp"
#include "game/input/control_behaviour.hpp"

namespace tanks {
class DirectionalLight;

class LightInput final : public ControlBehaviour {
	public:
	explicit LightInput(DirectionalLight &light);

	void updateControls(const PlayerInputs &inputs, float dt,
	                    World &world) override;

	void keyPressed(Key key);

	private:
	DirectionalLight &light_;
};

class DirectionalLight final : public GameObject {
	public:
	DirectionalLight();
	float elevation = 1.0f;
	float azimuth = .65f;
	bool castsShadows = true;
	LightInput input;
};

} // namespace tanks
