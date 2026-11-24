#include "game/render/models.hpp"
#include "game/render/draw_context.hpp"
#include "game/world/world.hpp"
#include <cmath>
#include <numeric>

namespace tanks {
void ArenaRenderer::draw(const World &world, DrawContext &draw) const {
	const float edge = world.config().rules.arenaHalfExtent;

	draw.arenaPrism(translate(0, -.26f, 0) * scale(edge, .52f, edge),
	                {.52f, .44f, .32f}, {.48f, 0.0f, 8.0f});

	const auto &outline = world.arena().outline();

	for (std::size_t i = 0; i < outline.size(); ++i) {
		const auto &a = outline[i];
		const auto &b = outline[(i + 1) % outline.size()];
		const float dx = b.x - a.x, dz = b.z - a.z;
		const float length = std::hypot(dx, dz);
		const float yaw = std::atan2(-dz, dx);
		const float outwardX = dz / length, outwardZ = -dx / length;
		const float x = std::midpoint(a.x, b.x) + outwardX * .48f;
		const float z = std::midpoint(a.z, b.z) + outwardZ * .48f;

		M4 wall = translate(x, 1.15f, z) * rotateY(yaw);

		draw.cube(wall * scale(length + .08f, 2.3f, .96f), {.31f, .34f, .35f},
		          {.3f, 0.0f, 8.0f});

		wall.a[13] = 2.32f;

		draw.cube(wall * scale(length + .08f, .1f, 1.08f), {.72f, .49f, .25f},
		          {.4f, 0.0f, 8.0f});

		const Vec2 pillar = a + normalized(a) * .6f;

		draw.cylinder(translate(pillar.x, 1.15f, pillar.z) *
		                  scale(1.05f, 2.3f, 1.05f),
		              {.24f, .29f, .31f}, {.4f, 0.0f, 8.0f});
	}
}
} // namespace tanks
