#pragma once
#include "core/color.hpp"
#include "game/command.hpp"
#include <string>
#include <vector>

namespace tanks {
struct TeamConfig {
	std::string name;
	Color color;
	std::vector<Controller> members;
};

struct TeamState {
	unsigned wins = 0;
	unsigned alive = 0;
};
} // namespace tanks
