#pragma once

namespace tanks {
enum class Phase { Ready, Countdown, Playing, RoundOver, MatchOver };
const char *phaseName(Phase phase);
} // namespace tanks
