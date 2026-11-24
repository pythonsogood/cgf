#pragma once
#include "game/match/team.hpp"
#include <cstdint>

namespace tanks {
struct Rules {
	float arenaHalfExtent = 19.0f;
	float tankRadius = 0.72f;
	float tankSpeed = 6.0f;
	float turretTurnSpeed = 5.0f;
	float maxHealth = 100.0f;
	float shellSpeed = 23.0f;
	float shellDamage = 38.0f;
	float splashRadius = 2.4f;
	float fireCooldown = 0.72f;
	float roundSeconds = 90.0f;
	float countdownSeconds = 2.0f;
	float intermissionSeconds = 4.0f;
	unsigned roundsToWin = 3;
	bool friendlyFire = false;
};

struct MatchConfig {
	Rules rules;
	std::vector<TeamConfig> teams;
	std::uint32_t seed = 2027;
};

MatchConfig freeForAllConfig();
} // namespace tanks
