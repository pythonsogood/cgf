#include "game/match/config.hpp"
#include "game/limits.hpp"
#include "game/match/validation.hpp"
#include <stdexcept>

namespace tanks {
MatchConfig freeForAllConfig() {
	MatchConfig c;
	c.teams = {{"CYAN", {0.15f, 0.9f, 0.95f}, {Controller::Human}},
	           {"RED", {1.0f, 0.28f, 0.20f}, {Controller::Bot}},
	           {"YELLOW", {1.0f, 0.82f, 0.16f}, {Controller::Bot}},
	           {"PURPLE", {0.68f, 0.35f, 0.95f}, {Controller::Bot}}};
	return c;
}

namespace detail {
namespace {
bool inRange(float value, float minimum, float maximum) {
	return value >= minimum && value <= maximum;
}

bool positiveUpTo(float value, float maximum) {
	return value > 0 && value <= maximum;
}
} // namespace
void validateConfig(const MatchConfig &config) {
	const Rules &r = config.rules;
	if (!inRange(r.arenaHalfExtent, 14, 20) ||
	    !inRange(r.tankRadius, 0.2f, 2.5f) || !positiveUpTo(r.tankSpeed, 30) ||
	    !positiveUpTo(r.turretTurnSpeed, 30) ||
	    !positiveUpTo(r.maxHealth, 10000) || !positiveUpTo(r.shellSpeed, 100) ||
	    !positiveUpTo(r.shellDamage, r.maxHealth * 10) ||
	    !inRange(r.splashRadius, 0, 10) || !positiveUpTo(r.fireCooldown, 30) ||
	    !positiveUpTo(r.roundSeconds, 3600) ||
	    !inRange(r.countdownSeconds, 0, 60) ||
	    !inRange(r.intermissionSeconds, 0, 60) || r.roundsToWin == 0 ||
	    config.teams.size() < 2 || config.teams.size() > maxTeams)
		throw std::invalid_argument("invalid tanks match rules or team count");

	std::size_t memberCount = 0;
	std::size_t humanCount = 0;

	for (const TeamConfig &team : config.teams) {
		if (team.members.empty() || team.name.empty() ||
		    !inRange(team.color.r, 0, 1) || !inRange(team.color.g, 0, 1) ||
		    !inRange(team.color.b, 0, 1))
			throw std::invalid_argument("invalid team roster or color");

		memberCount += team.members.size();

		for (Controller controller : team.members)
			humanCount += controller == Controller::Human;
	}

	if (memberCount > maxTanks)
		throw std::invalid_argument("roster exceeds maximum tank count");

	if (humanCount > 1)
		throw std::invalid_argument("roster has multiple human players");
}
} // namespace detail
} // namespace tanks
