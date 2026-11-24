#include "core/object/transform.hpp"

namespace tanks {
void Transform::setYaw(float radians) { setForward(direction(radians)); }

void Transform::setForward(Vec2 unitDirection) {
	axisX_ = {unitDirection.z, -unitDirection.x};
}

Vec2 Transform::toLocalVector(Vec2 offset) const {
	return {dot(offset, axisX_), dot(offset, axisZ())};
}

Vec2 Transform::toLocalPoint(Vec2 point) const {
	return toLocalVector(point - position);
}

Vec2 Transform::toWorldPoint(Vec2 local) const {
	return position + axisX_ * local.x + axisZ() * local.z;
}
} // namespace tanks
