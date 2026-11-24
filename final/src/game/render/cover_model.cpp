#include "game/render/models.hpp"
#include "game/render/draw_context.hpp"
#include "game/physics/collider.hpp"
#include "core/object/game_object.hpp"
#include <initializer_list>

namespace tanks {
void CoverRenderer::draw(const World &, DrawContext &draw) const {
	const GameObject &object = owner();

	const auto *collider = owner().getComponent<BoxCollider>();
	if (!collider)
		return;

	const M4 base = modelMatrix(object.transform());
	const float height = collider->height;

	draw.cube(
	    base * translate(0, height * .5f, 0) *
	        scale(collider->halfSize.x * 2, height, collider->halfSize.z * 2),
	    {.42f, .39f, .34f}, {.34f, 0.0f, 8.0f});

	draw.cube(base * translate(0, height + .06f, 0) *
	              scale(collider->halfSize.x * 2, .12f, collider->halfSize.z * 2),
	          {.59f, .51f, .39f}, {.35f, 0.0f, 8.0f});

	if (collider->halfSize.x > 1.5f && collider->halfSize.z > 1.5f)
		for (float x : {-0.5f, 0.5f})
			draw.cube(base * translate(x * collider->halfSize.x * 2, height * .5f, 0) *
			              scale(.1f, height, collider->halfSize.z * 2),
			          {.27f, .27f, .25f}, {.3f, 0.0f, 8.0f});
}
} // namespace tanks
