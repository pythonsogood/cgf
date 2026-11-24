#include "game/world/world.hpp"
#include "game/physics/collision.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace tanks {
namespace {
void resetControls(GameObject &object) {
	for (Component *component = object.firstComponent(); component;
	     component = component->nextComponent())
		if (auto *control = dynamic_cast<ControlBehaviour *>(component))
			control->resetControls();
}

void checkSpawnCollider(const BoxCollider &collider, const Vec2 &candidate,
                        const float &radius, bool &clear) {
	if (clear && detail::circleBox(candidate, radius, collider))
		clear = false;
}
} // namespace
void World::resetRound() {
	for (TeamState &t : match_.teams)
		t.alive = 0;

	for (Tank &t : tanks_) {
		t.setActive(true);
		t.health.reset();
		t.motion.velocity = {};

		const float base = 2.0f * pi * static_cast<float>(t.control.team) /
		                       static_cast<float>(config_.teams.size()) +
		                   pi * 0.5f;
		const Vec2 radial = direction(base), tangent{radial.z, -radial.x},
		           origin = radial * (config_.rules.arenaHalfExtent * 0.64f);
		const float spacing = std::max(1.7f, t.collider.radius * 2.0f + 0.05f);

		bool placed = false;

		for (int ring = 0; ring <= 12 && !placed; ++ring)
			for (int distance = ring; distance <= 2 * ring && !placed; ++distance)
				for (int x = -ring; x <= ring && !placed; ++x)
					for (int z = -ring; z <= ring && !placed; ++z) {
						if (std::max(std::abs(x), std::abs(z)) != ring ||
						    std::abs(x) + std::abs(z) != distance)
							continue;

						const Vec2 candidate = origin +
						                       tangent * (static_cast<float>(x) * spacing) +
						                       radial * (static_cast<float>(z) * spacing);

						if (!arena_.contains(candidate, t.collider.radius))
							continue;

						bool clear = true;
						forEachBoxCollider(checkSpawnCollider, candidate,
						                   t.collider.radius + 0.05f, clear);

						for (std::size_t other = 0; other < t.id && clear; ++other) {
							const Tank &prior = tanks_[other];
							const Vec2 delta = candidate - prior.transform().position;
							if (dot(delta, delta) < spacing * spacing)
								clear = false;
						}

						if (clear) {
							t.transform().position = candidate;
							placed = true;
						}
					}

		if (!placed)
			throw std::invalid_argument("arena cannot fit tank spawn roster");

		t.transform().setYaw(base + pi);
		t.weapon.reset(base + pi);
		++match_.teams[t.control.team].alive;

		resetControls(t);
	}

	for (GameObject &object : genericObjects_)
		resetControls(object);

	for (Projectile &p : combat_.projectiles)
		p.setActive(false);

	for (Effect &e : combat_.effects)
		e.setActive(false);

	match_.roundTime = config_.rules.roundSeconds;
	++match_.round;
	match_.roundWinner = noTeam;
}

void World::finishRound(TeamId winner) {
	match_.roundWinner = winner;
	if (winner != noTeam)
		++match_.teams[winner].wins;

	match_.phase = Phase::RoundOver;
	match_.phaseTime = 0.0f;

	if (winner != noTeam && match_.teams[winner].wins >= config_.rules.roundsToWin)
		match_.matchWinner = winner;
}

const char *phaseName(Phase phase) {
	switch (phase) {
	case Phase::Ready:
		return "Ready";
	case Phase::Countdown:
		return "Countdown";
	case Phase::Playing:
		return "Playing";
	case Phase::RoundOver:
		return "Round Over";
	case Phase::MatchOver:
		return "Match Over";
	}
	return "Unknown";
}
} // namespace tanks
