#include "core/object/game_object.hpp"
#include "game/physics/collision.hpp"
#include <algorithm>
#include <cmath>
namespace tanks::detail {
namespace {
Vec2 closestLocalPoint(Vec2 point, Vec2 halfSize) {
	return {std::clamp(point.x, -halfSize.x, halfSize.x),
	        std::clamp(point.z, -halfSize.z, halfSize.z)};
}
} // namespace

bool finite(Vec2 v) { return std::isfinite(v.x) && std::isfinite(v.z); }

Vec2 closestPoint(Vec2 p, const BoxCollider &collider) {
	const Transform &transform = collider.owner().transform();
	const Vec2 local = transform.toLocalPoint(p);
	return transform.toWorldPoint(closestLocalPoint(local, collider.halfSize));
}

bool circleBox(Vec2 p, float r, const BoxCollider &collider) {
	const Vec2 q = closestPoint(p, collider);
	return dot(p - q, p - q) < r * r;
}

Vec2 pushOutOfBox(Vec2 point, float radius, const BoxCollider &collider) {
	const Transform &transform = collider.owner().transform();
	const Vec2 half = collider.halfSize;
	Vec2 local = transform.toLocalPoint(point);
	const Vec2 separation = local - closestLocalPoint(local, half);
	const float distance = length(separation);

	if (distance >= radius)
		return point;

	if (distance > epsilon) {
		local += separation * ((radius - distance) / distance);
	} else {
		const float left = local.x + half.x, right = half.x - local.x;
		const float back = local.z + half.z, front = half.z - local.z;
		const float nearest = std::min({left, right, back, front});

		if (nearest == left)
			local.x = -half.x - radius;
		else if (nearest == right)
			local.x = half.x + radius;
		else if (nearest == back)
			local.z = -half.z - radius;
		else
			local.z = half.z + radius;
	}

	return transform.toWorldPoint(local);
}

bool segmentBox(Vec2 a, Vec2 b, const BoxCollider &collider, float radius,
                float *entry) {
	const Transform &transform = collider.owner().transform();
	const Vec2 localStart = transform.toLocalPoint(a);
	const Vec2 localDelta = transform.toLocalVector(b - a);
	const float minX = -collider.halfSize.x - radius,
	            maxX = collider.halfSize.x + radius,
	            minZ = -collider.halfSize.z - radius,
	            maxZ = collider.halfSize.z + radius;
	const float origins[2] = {localStart.x, localStart.z},
	            dirs[2] = {localDelta.x, localDelta.z}, mins[2] = {minX, minZ},
	            maxs[2] = {maxX, maxZ};
	float lo = 0.0f, hi = 1.0f;

	for (int axis = 0; axis < 2; ++axis) {
		if (std::abs(dirs[axis]) < epsilon) {
			if (origins[axis] < mins[axis] || origins[axis] > maxs[axis])
				return false;
		} else {
			float t0 = (mins[axis] - origins[axis]) / dirs[axis],
			      t1 = (maxs[axis] - origins[axis]) / dirs[axis];
			if (t0 > t1)
				std::swap(t0, t1);
			lo = std::max(lo, t0);
			hi = std::min(hi, t1);
			if (lo > hi)
				return false;
		}
	}

	if (hi < 0.0f || lo > 1.0f)
		return false;

	if (entry)
		*entry = std::max(0.0f, lo);

	return true;
}
} // namespace tanks::detail
