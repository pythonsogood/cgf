#include "game/world/directional_light.hpp"
#include "game/input/keyboard.hpp"
#include "core/math.hpp"

namespace tanks {
LightInput::LightInput(DirectionalLight &light) : light_(light) {}

void LightInput::updateControls(const PlayerInputs &inputs, float dt, World &) {
	if (inputs.keyboard.isDown('l'))
		light_.azimuth = wrapAngle(light_.azimuth + .65f * dt);
}

void LightInput::keyPressed(Key key) {
	if (light_.active() && key == Key::F3)
		light_.castsShadows = !light_.castsShadows;
}
DirectionalLight::DirectionalLight() : input(*this) { attachComponent(input); }
} // namespace tanks
