#include "game/combat/effect.hpp"

namespace tanks {
Effect::Effect() {
	transform().elevation = .12f;
	setActive(false);
	attachComponent(effect);
	attachComponent(mesh_);
}
} // namespace tanks
