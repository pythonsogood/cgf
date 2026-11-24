#include "game/world/world.hpp"
#include "game/input/control_behaviour.hpp"
#include "game/match/validation.hpp"
#include "game/physics/collision.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace tanks {
namespace {
float validatedArenaRadius(const MatchConfig &config) {
	detail::validateConfig(config);
	return config.rules.arenaHalfExtent;
}

void testLineOfSight(const BoxCollider &collider, const Vec2 &from,
                     const Vec2 &to, bool &blocked) {
	if (!blocked && detail::segmentBox(from, to, collider))
		blocked = true;
}
} // namespace
void updateControls(GameObject &object, const PlayerInputs &inputs, float dt,
                    World &world) {
	if (!object.active())
		return;

	for (Component *component = object.firstComponent(); component;
	     component = component->nextComponent())
		if (auto *control = dynamic_cast<ControlBehaviour *>(component))
			control->updateControls(inputs, dt, world);
}

World::World(MatchConfig config)
    : config_(std::move(config)), arena_(validatedArenaRadius(config_)),
      botRandom_(config_.seed), effectRandom_(config_.seed) {
	const Rules &r = config_.rules;

	for (TeamId team = 0; team < config_.teams.size(); ++team)
		for (Controller controller : config_.teams[team].members) {
			if (controller != Controller::Human && controller != Controller::Bot)
				throw std::invalid_argument("invalid tank controller");
			tanks_.emplace_back(tanks_.size(), team, controller, r);
		}

	resetRound();

	match_.phase = Phase::Ready;
	match_.phaseTime = 0.0f;
}

float World::randomUnit() {
	return std::uniform_real_distribution<float>(0.0f, 1.0f)(botRandom_);
}

bool World::lineOfSight(Vec2 from, Vec2 to) const {
	bool blocked = false;
	forEachBoxCollider(testLineOfSight, from, to, blocked);
	return !blocked;
}

void World::start() {
	if (match_.phase != Phase::Ready)
		return;

	match_.phase = Phase::Countdown;
	match_.phaseTime = 0.0f;
}

void World::restart() {
	for (TeamState &t : match_.teams)
		t = {};

	match_.round = 0;
	match_.roundWinner = noTeam;
	match_.matchWinner = noTeam;

	combat_.shotsFired = combat_.damageEvents = 0;

	botRandom_.seed(config_.seed);
	effectRandom_.seed(config_.seed);

	resetRound();

	match_.phase = Phase::Countdown;
	match_.phaseTime = 0.0f;
}

void World::step(float dt, const PlayerInputs &inputs) {
	if (!std::isfinite(dt) || dt < 0.0f || dt > 0.03f)
		throw std::invalid_argument("step dt must be finite and in [0, 0.03]");

	for (Effect &effect : combat_.effects)
		if (effect.active()) {
			Blast &blast = effect.effect;
			blast.age += dt;

			if (blast.age >= blast.duration)
				effect.setActive(false);
		}

	if (dt == 0.0f || match_.phase == Phase::Ready ||
	    match_.phase == Phase::MatchOver)
		return;

	match_.phaseTime += dt;

	if (match_.phase == Phase::Countdown) {
		if (match_.phaseTime >= config_.rules.countdownSeconds) {
			match_.phase = Phase::Playing;
			match_.phaseTime = 0.0f;
		}

		return;
	}

	if (match_.phase == Phase::RoundOver) {
		if (match_.matchWinner != noTeam) {
			match_.phase = Phase::MatchOver;
			return;
		}

		if (match_.phaseTime >= config_.rules.intermissionSeconds) {
			resetRound();

			match_.phase = Phase::Countdown;
			match_.phaseTime = 0.0f;
		}

		return;
	}

	if (match_.phase != Phase::Playing)
		return;

	for (Tank &tank : tanks_)
		tanks::updateControls(tank, inputs, dt, *this);

	for (GameObject &object : genericObjects_)
		tanks::updateControls(object, inputs, dt, *this);

	for (std::size_t i = 0; i < tanks_.size(); ++i)
		for (std::size_t j = i + 1; j < tanks_.size(); ++j) {
			Tank &a = tanks_[i], &b = tanks_[j];
			if (!a.active() || !b.active())
				continue;

			Vec2 delta = b.transform().position - a.transform().position;
			float d = length(delta);
			const float minD = a.collider.radius + b.collider.radius;

			if (d < minD) {
				const Vec2 n = d > detail::epsilon ? delta / d : Vec2{1, 0};
				const Vec2 push = n * ((minD - d) * 0.5f);

				a.motion.displace(push * -1.0f, *this);
				b.motion.displace(push, *this);
			}
		}

	updateProjectiles(dt);

	match_.roundTime = std::max(0.0f, match_.roundTime - dt);

	TeamId survivor = noTeam;
	unsigned livingTeams = 0;

	for (TeamId i = 0; i < config_.teams.size(); ++i)
		if (match_.teams[i].alive) {
			survivor = i;
			++livingTeams;
		}

	if (livingTeams <= 1) {
		finishRound(livingTeams == 1 ? survivor : noTeam);
		return;
	}

	if (match_.roundTime <= 0.0f) {
		unsigned bestCount = 0;
		float bestHealth = -1.0f;
		TeamId winner = noTeam;
		bool tied = false;

		for (TeamId team = 0; team < config_.teams.size(); ++team) {
			unsigned count = 0;
			float health = 0;

			for (const Tank &tank : tanks_)
				if (tank.active() && tank.control.team == team) {
					++count;
					health += tank.health.current;
				}

			if (count > bestCount ||
			    (count == bestCount && health > bestHealth + detail::epsilon)) {
				bestCount = count;
				bestHealth = health;
				winner = team;
				tied = false;
			} else if (count == bestCount &&
			           std::abs(health - bestHealth) <= detail::epsilon)
				tied = true;
		}

		finishRound(tied ? noTeam : winner);
	}
}
} // namespace tanks
