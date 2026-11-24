#include "game/physics/movement.hpp"
#include "game/world/world.hpp"
#include "game/physics/collision.hpp"
namespace tanks {
namespace {
void pushOutOfCollider(const BoxCollider &collider, Vec2 &position,
                       float radius) {
	position = detail::pushOutOfBox(position, radius, collider);
}
} // namespace

void TankMotion::move(Vec2 direction, float dt, const World &world) {
	direction = detail::finite(direction) ? normalized(direction) : Vec2{};

	velocity = direction * speed_;

	displace(velocity * dt, world);

	if (direction.x != 0 || direction.z != 0)
		owner().transform().setForward(direction);
}

void TankMotion::displace(Vec2 offset, const World &world) {
	Transform &transform = owner().transform();
	Vec2 next = transform.position + offset;
	const float radius = collider_.radius;

	for (int pass = 0; pass < 32; ++pass) {
		const Vec2 before = next;

		next = world.arena().confine(next, radius);
		world.forEachBoxCollider(pushOutOfCollider, next, radius);

		if (next.x == before.x && next.z == before.z)
			break;
	}
	transform.position = next;
}
} // namespace tanks
