#pragma once
#include "core/color.hpp"
#include "game/identity.hpp"
#include "game/world/world.hpp"

namespace tanks {
Color teamColor(const World &world, TeamId team);
} // namespace tanks
