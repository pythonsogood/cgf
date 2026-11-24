#pragma once
#include "core/color.hpp"
#include "core/model_matrix.hpp"
#include "core/object/transform.hpp"
#include "game/render/material.hpp"

namespace tanks {
class DrawContext {
	public:
	bool shadowPass() const { return depthPass_; }
	virtual void cube(const M4 &model, Color color, Material material) = 0;
	virtual void cylinder(const M4 &model, Color color, Material material) = 0;
	virtual void arenaPrism(const M4 &model, Color color, Material material) = 0;

	protected:
	~DrawContext() = default;
	bool depthPass_ = false;
};

M4 modelMatrix(const Transform &transform);
} // namespace tanks
