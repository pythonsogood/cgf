#include "game/combat/projectile.hpp"

namespace tanks {
Projectile::Projectile() {
	setActive(false);
	attachComponent(flight);
	attachComponent(mesh_);
}
} // namespace tanks
