#pragma once
#include "game/world/arena.hpp"
#include "game/world/directional_light.hpp"
#include "game/match/config.hpp"
#include "game/limits.hpp"
#include "game/match/match.hpp"
#include "game/combat/projectile.hpp"
#include "game/tank.hpp"
#include "game/match/state.hpp"
#include "game/combat/state.hpp"
#include "game/match/team.hpp"
#include <array>
#include <cstdint>
#include <deque>
#include <list>
#include <random>

namespace tanks {
class World {
	public:
	explicit World(MatchConfig config);
	const MatchConfig &config() const { return config_; }

	const std::deque<Tank> &tanks() const { return tanks_; }

	Tank &tank(TankId id) { return tanks_.at(id); }

	const Arena &arena() const { return arena_; }

	DirectionalLight &light() { return light_; }

	const DirectionalLight &light() const { return light_; }

	const std::array<Projectile, maxProjectiles> &projectiles() const {
		return combat_.projectiles;
	}

	const std::array<TeamState, maxTeams> &teams() const { return match_.teams; }

	Phase phase() const { return match_.phase; }

	float phaseTime() const { return match_.phaseTime; }

	float roundTime() const { return match_.roundTime; }

	unsigned round() const { return match_.round; }

	TeamId roundWinner() const { return match_.roundWinner; }

	TeamId matchWinner() const { return match_.matchWinner; }

	std::uint64_t shotsFired() const { return combat_.shotsFired; }

	std::uint64_t damageEvents() const { return combat_.damageEvents; }

	GameObject &createObject() { return genericObjects_.emplace_back(); }

	template <class Visitor, class... Args>
	void forEachObject(const Visitor &visit, Args &&...args) const {
		visit(light_, args...);
		visit(arena_, args...);

		for (const GameObject &object : arena_.obstacles())
			visit(object, args...);

		for (const GameObject &object : tanks_)
			visit(object, args...);

		for (const GameObject &object : combat_.projectiles)
			visit(object, args...);

		for (const GameObject &object : combat_.effects)
			visit(object, args...);

		for (const GameObject &object : genericObjects_)
			visit(object, args...);
	}

	template <class Visitor, class... Args>
	void forEachBoxCollider(const Visitor &visit, Args &&...args) const {
		for (const Obstacle &object : arena_.obstacles())
			if (object.active())
				visit(object.collider, args...);

		for (const GameObject &object : genericObjects_) {
			if (!object.active())
				continue;

			if (const BoxCollider *collider = object.getComponent<BoxCollider>())
				visit(*collider, args...);
		}
	}

	void start();

	void restart();

	void step(float dt, const PlayerInputs &inputs = {});

	bool lineOfSight(Vec2 from, Vec2 to) const;

	bool spawnShell(const Tank &tank);

	Vec2 predictedImpact(const Tank &tank) const;

	float randomUnit();

	private:
	MatchConfig config_;
	Arena arena_;
	DirectionalLight light_;
	std::deque<Tank> tanks_;
	std::list<GameObject> genericObjects_;
	detail::MatchState match_;
	detail::CombatState combat_;
	std::minstd_rand botRandom_, effectRandom_;

	void resetRound();

	void finishRound(TeamId winner);

	void updateProjectiles(float dt);

	void explode(Vec2 position, TeamId team, TankId directHit);

	void addEffect(Vec2 position, Color color, float radius, float duration);

	Vec2 shellMuzzle(const Tank &tank, Vec2 direction) const;

	bool firstProjectileHit(Vec2 start, Vec2 end, TankId owner, TeamId team,
	                        float &entry, TankId &hitTank) const;
};
} // namespace tanks
