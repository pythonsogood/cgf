#pragma once
#include <cstddef>

namespace tanks {
constexpr std::size_t maxTeams = 8;
constexpr std::size_t maxTanks = 32;
constexpr std::size_t maxProjectiles = 128;
constexpr std::size_t maxEffects = 96;
constexpr float fixedStep = 1.0f / 120.0f;
} // namespace tanks
