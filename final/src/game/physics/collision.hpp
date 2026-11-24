#pragma once
#include "game/physics/collider.hpp"
#include "core/math.hpp"

namespace tanks::detail {
constexpr float epsilon = 1.0e-6f;

bool finite(Vec2 value);

Vec2 closestPoint(Vec2 point, const BoxCollider &collider);

bool circleBox(Vec2 point, float radius, const BoxCollider &collider);

Vec2 pushOutOfBox(Vec2 point, float radius, const BoxCollider &collider);

bool segmentBox(Vec2 start, Vec2 end, const BoxCollider &collider,
                float radius = 0.0f, float *entry = nullptr);
} // namespace tanks::detail
