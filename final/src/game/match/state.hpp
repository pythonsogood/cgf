#pragma once
#include "game/match/match.hpp"
#include "game/match/team.hpp"
#include "game/identity.hpp"
#include <array>

namespace tanks::detail {
struct MatchState {
	Phase phase = Phase::Ready;
	float phaseTime = 0.0f;
	float roundTime = 0.0f;
	unsigned round = 0;
	TeamId roundWinner = noTeam;
	TeamId matchWinner = noTeam;
	std::array<TeamState, maxTeams> teams{};
};
} // namespace tanks::detail
