#include "game/world/world.hpp"
#include "game/physics/collision.hpp"
#include <algorithm>
#include <cmath>
#include <random>

namespace tanks {
namespace {
constexpr float projectileLife = 4.0f;
float sqr(float v) { return v * v; }
void testProjectileBox(const BoxCollider &collider, const Vec2 &start,
                       const Vec2 &end, float &entry, bool &hit,
                       TankId &hitTank) {
	float candidate = 0.0f;

	if (detail::segmentBox(start, end, collider, 0.06f, &candidate) &&
	    candidate < entry) {
		entry = candidate;
		hit = true;
		hitTank = noTank;
	}
}

float effectAge(const Effect &effect) { return effect.effect.age; }
} // namespace

void World::addEffect(Vec2 position, Color color, float radius,
                      float duration) {
	auto &effects = combat_.effects;
	auto slot = std::ranges::find(effects, false, &Effect::active);
	if (slot == effects.end())
		slot = std::ranges::max_element(effects, {}, effectAge);

	Effect &effect = *slot;
	effect.transform().position = position;
	effect.effect.color = color;
	effect.effect.age = 0.0f;
	effect.effect.duration = duration;

	std::uniform_real_distribution<float> unit(0.0f, 1.0f);
	for (Spark &spark : effect.effect.sparks) {
		const float angle = unit(effectRandom_) * 2 * pi;
		spark.velocity = direction(angle) *
		                 ((0.3f + 0.7f * unit(effectRandom_)) * radius / duration);
		spark.lifetime = duration * (0.5f + 0.5f * unit(effectRandom_));
		spark.lift = 2.0f + 4.0f * unit(effectRandom_);
		spark.size = 0.09f + 0.13f * unit(effectRandom_);
	}

	effect.setActive(true);
}

Vec2 World::shellMuzzle(const Tank &tank, Vec2 dir) const {
	return arena_.confine(
	    tank.transform().position + dir * (tank.collider.radius + 0.24f), 0.06f);
}

bool World::spawnShell(const Tank &tank) {
	if (!tank.active())
		return false;

	for (Projectile &p : combat_.projectiles)
		if (!p.active()) {
			const Vec2 dir = direction(tank.weapon.turretAngle);
			p.transform().position = shellMuzzle(tank, dir);
			p.transform().elevation = tank.transform().elevation + barrelHeight;
			p.transform().setForward(dir);

			p.flight.velocity = dir * config_.rules.shellSpeed;
			p.flight.team = tank.control.team;
			p.flight.owner = tank.id;
			p.flight.life = projectileLife;
			p.setActive(true);

			++combat_.shotsFired;

			return true;
		}

	return false;
}

Vec2 World::predictedImpact(const Tank &tank) const {
	const Vec2 dir = direction(tank.weapon.turretAngle);
	const Vec2 start = shellMuzzle(tank, dir);
	const Vec2 end = start + dir * (config_.rules.shellSpeed * projectileLife);
	float entry = 1.0f;
	TankId hitTank = noTank;

	if (firstProjectileHit(start, end, tank.id, tank.control.team, entry, hitTank))
		return start + (end - start) * entry;

	return end;
}

bool World::firstProjectileHit(Vec2 start, Vec2 end, TankId owner, TeamId team,
                               float &entry, TankId &hitTank) const {
	entry = 1.0f;
	bool hit = false;
	hitTank = noTank;

	forEachBoxCollider(testProjectileBox, start, end, entry, hit, hitTank);

	float boundaryEntry = 1.0f;

	if (arena_.traceBoundary(start, end, 0.06f, boundaryEntry) &&
	    boundaryEntry < entry) {
		entry = boundaryEntry;
		hit = true;
		hitTank = noTank;
	}

	const Vec2 delta = end - start;
	const float a = dot(delta, delta);

	for (const Tank &t : tanks_) {
		if (!t.active() || t.id == owner ||
		    (!config_.rules.friendlyFire && t.control.team == team))
			continue;

		const Vec2 relative = start - t.transform().position;
		const float combinedRadius = t.collider.radius + 0.08f;
		const float c = dot(relative, relative) - sqr(combinedRadius);

		if (c <= 0.0f) {
			if (0.0f < entry) {
				entry = 0.0f;
				hit = true;
				hitTank = t.id;
			}
			continue;
		}

		if (a <= detail::epsilon)
			continue;

		const float b = dot(relative, delta);
		const float discriminant = b * b - a * c;

		if (discriminant < 0.0f)
			continue;

		const float candidate = (-b - std::sqrt(discriminant)) / a;

		if (candidate >= 0.0f && candidate <= 1.0f && candidate < entry) {
			entry = candidate;
			hit = true;
			hitTank = t.id;
		}
	}

	return hit;
}

void World::explode(Vec2 position, TeamId team, TankId directHit) {
	Color color = team < config_.teams.size() ? config_.teams[team].color
	                                          : Color{1, 0.6f, 0.1f};

	addEffect(position, color, config_.rules.splashRadius, 0.42f);

	for (Tank &t : tanks_) {
		if (!t.active() || (!config_.rules.friendlyFire && t.control.team == team))
			continue;

		const float d = length(t.transform().position - position);
		if (d > config_.rules.splashRadius && t.id != directHit)
			continue;

		const float damage =
		    t.id == directHit
		        ? config_.rules.shellDamage
		        : config_.rules.shellDamage *
		              std::max(0.0f, 1.0f - d / std::max(config_.rules.splashRadius,
		                                                 detail::epsilon));
		if (damage <= 0)
			continue;

		const bool killed = t.health.takeDamage(damage);
		++combat_.damageEvents;

		if (killed) {
			t.setActive(false);
			t.motion.velocity = {};

			if (t.control.team < match_.teams.size() &&
			    match_.teams[t.control.team].alive)
				--match_.teams[t.control.team].alive;

			addEffect(t.transform().position, config_.teams[t.control.team].color, 1.3f,
			          0.7f);
		}
	}
}

void World::updateProjectiles(float dt) {
	for (Projectile &p : combat_.projectiles)
		if (p.active()) {
			const Vec2 start = p.transform().position;
			const float travelTime = std::min(dt, p.flight.life);
			const Vec2 end = start + p.flight.velocity * travelTime;
			float earliest = 1.0f;
			TankId hitTank = noTank;
			const bool hit = firstProjectileHit(start, end, p.flight.owner,
			                                    p.flight.team, earliest, hitTank);

			p.flight.life -= dt;

			if (hit || p.flight.life <= 0.0f) {
				const Vec2 position = hit ? start + (end - start) * earliest : end;

				p.transform().position = position;
				explode(position, p.flight.team, hitTank);
				p.setActive(false);
			} else
				p.transform().position = end;
		}
}
} // namespace tanks
