#include "game/render/draw_context.hpp"

namespace tanks {
M4 modelMatrix(const Transform &transform) {
	const Vec2 axis = transform.axisX();

	M4 model =
	    translate(transform.position.x, transform.elevation, transform.position.z);

	model.a[0] = axis.x;
	model.a[2] = axis.z;
	model.a[8] = -axis.z;
	model.a[10] = axis.x;

	return model;
}
} // namespace tanks
