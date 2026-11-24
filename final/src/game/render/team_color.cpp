#include "game/render/team_color.hpp"

namespace tanks {
Color teamColor(const World &world, TeamId team) {
	static constexpr Color defaults[] = {
	    {.14f, .83f, .96f}, {1, .38f, .32f}, {1, .76f, .25f}, {.72f, .43f, 1}};

	if (team < world.config().teams.size())
		return world.config().teams[team].color;

	return defaults[team % 4];
}
} // namespace tanks
