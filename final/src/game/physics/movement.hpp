#pragma once
#include "game/physics/collider.hpp"

namespace tanks {
class World;

class TankMotion final : public Component {
	public:
	TankMotion(float speed, const CircleCollider &collider)
	    : speed_(speed), collider_(collider) {}

	void move(Vec2 direction, float dt, const World &world);

	void displace(Vec2 offset, const World &world);
	Vec2 velocity;

	private:
	float speed_;
	const CircleCollider &collider_;
};
} // namespace tanks
