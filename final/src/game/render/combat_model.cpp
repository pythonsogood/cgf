#include "game/render/models.hpp"
#include "game/render/draw_context.hpp"
#include "game/combat/effect.hpp"
#include "game/combat/projectile.hpp"
#include "game/render/team_color.hpp"
#include "game/world/world.hpp"

namespace tanks {
void ShellRenderer::draw(const World &world, DrawContext &draw) const {
	if (draw.shadowPass())
		return;

	const auto *flight = owner().getComponent<ProjectileFlight>();
	if (!flight)
		return;

	const M4 base = modelMatrix(owner().transform());

	draw.cylinder(base * rotateX(pi * .5f) * scale(.2f, .38f, .2f),
	              teamColor(world, flight->team), {.8f, 0.0f, 8.0f});

	draw.cube(base * translate(0, 0, -.4f) * scale(.07f, .07f, .55f),
	          {1, .72f, .3f}, {.8f, 0.0f, 8.0f});
}

void BlastRenderer::draw(const World &, DrawContext &draw) const {
	if (draw.shadowPass())
		return;

	const auto *blast = owner().getComponent<Blast>();
	if (!blast || blast->duration <= 0)
		return;

	const Transform &origin = owner().transform();
	const float t = blast->age;

	for (const Spark &spark : blast->sparks) {
		if (t >= spark.lifetime)
			continue;

		const Vec2 point = origin.position + spark.velocity * t;
		const float height = origin.elevation + .15f + spark.lift * t - 4.9f * t * t;
		if (height <= 0)
			continue;

		const float size = spark.size * (1 - t / spark.lifetime);
		draw.cube(translate(point.x, height, point.z) * scale(size, size, size),
		          blast->color, {1.0f, 0.0f, 8.0f});
	}
}
} // namespace tanks
