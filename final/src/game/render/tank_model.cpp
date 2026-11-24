#include "game/render/models.hpp"
#include "game/render/draw_context.hpp"
#include "game/combat/projectile.hpp"
#include "game/tank.hpp"
#include "game/render/team_color.hpp"
#include "game/world/world.hpp"

namespace tanks {
void TankRenderer::draw(const World &world, DrawContext &draw) const {
	const GameObject &object = owner();
	const auto *control = object.getComponent<TankControl>();
	const Color color = teamColor(world, control ? control->team : noTeam);
	const Transform &transform = object.transform();
	const Vec2 position = transform.position;
	const M4 base = modelMatrix(transform);
	const M4 hullTransform = base * translate(0, .48f, 0);

	draw.cube(hullTransform * scale(1.5f, .55f, 1.9f), {.15f, .18f, .21f},
	          {.26f, 0.0f, 8.0f});

	for (float side : {-1.0f, 1.0f}) {
		draw.cube(hullTransform * translate(side * .73f, .02f, 0) *
		              scale(.28f, .42f, 1.85f),
		          {.09f, .12f, .15f}, {.22f, 0.0f, 8.0f});

		for (int k = -2; k <= 2; ++k)
			draw.cylinder(hullTransform * translate(side * .76f, -.12f, k * .32f) *
			                  rotateZ(pi * .5f) * scale(.34f, .3f, .34f),
			              {.34f, .39f, .4f}, {.25f, .0f, 8.0f});
	}

	draw.cube(hullTransform * translate(0, .38f, 0) * scale(1.18f, .44f, 1.16f),
	          color, {.30f, .18f, 48.0f});

	const auto *weapon = object.getComponent<TankWeapon>();
	const M4 turretPivot =
	    weapon ? translate(position.x, barrelHeight + transform.elevation,
					                   position.z) *
	                 rotateY(weapon->turretAngle)
					       : base * translate(0, barrelHeight, 0);

	draw.cylinder(turretPivot * translate(0, -.04f, 0) * scale(.86f, .34f, .86f),
	              color, {.30f, .18f, 48.0f});

	constexpr float barrelCenter = .73f, barrelLength = .8f;
	constexpr float muzzleOffset = barrelCenter + barrelLength * .5f;

	draw.cube(turretPivot * translate(0, 0, barrelCenter) *
	              scale(.2f, .2f, barrelLength),
	          {.22f, .25f, .27f}, {.28f, .08f, 24.0f});

	const M4 muzzleTransform = turretPivot * translate(0, 0, muzzleOffset);

	if (!draw.shadowPass() && control &&
	    control->controller == Controller::Human) {
		if (const auto *tank = dynamic_cast<const Tank *>(&object)) {
			const Vec2 forward{turretPivot.a[8], turretPivot.a[10]};
			const float reach =
			    dot(world.predictedImpact(*tank) - position, forward) - muzzleOffset;

			if (reach > 0) {
				const Color guide{.45f + .55f * color.r, .45f + .55f * color.g,
				                  .45f + .55f * color.b};

				draw.cube(muzzleTransform * translate(0, 0, reach * .5f) *
				              scale(.075f, .045f, reach),
				          guide, {1.0f, 0.0f, 8.0f});
			}
		}
	}

	if (!draw.shadowPass() && weapon && weapon->muzzleFlash > 0)
		draw.cylinder(muzzleTransform * scale(.3f, .3f, .3f), {1, .74f, .25f},
		              {.8f, .0f, 8.0f});
}
} // namespace tanks
