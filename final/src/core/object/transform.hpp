#pragma once
#include "core/math.hpp"
#include "core/object/component.hpp"

namespace tanks {
struct Transform final : Component {
	Vec2 position;
	float elevation = 0.0f;

	Vec2 axisX() const { return axisX_; }

	void setYaw(float radians);

	void setForward(Vec2 unitDirection);

	Vec2 toLocalVector(Vec2 offset) const;

	Vec2 toLocalPoint(Vec2 point) const;

	Vec2 toWorldPoint(Vec2 local) const;

	private:
	Vec2 axisZ() const { return {-axisX_.z, axisX_.x}; }
	Vec2 axisX_{1.0f, 0.0f};
};
} // namespace tanks
