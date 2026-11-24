#pragma once
#include "core/object/component.hpp"

namespace tanks {
class World;
class DrawContext;

class MeshRenderer : public Component {
	public:
	virtual void draw(const World &world, DrawContext &context) const = 0;
	bool visible = true;
};
} // namespace tanks
