#pragma once
#include "game/limits.hpp"

namespace tanks {
using TeamId = std::size_t;
using TankId = std::size_t;
constexpr TeamId noTeam = maxTeams;
constexpr TankId noTank = maxTanks;
} // namespace tanks
