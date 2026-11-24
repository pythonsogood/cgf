#include "app/runtime/timing.hpp"
#include <algorithm>

namespace tanks::app {
void LoopTiming::reset() {
	lastTick_ = Clock::now();
	accumulator = 0;
}

double LoopTiming::beginTick() {
	const auto now = Clock::now();
	const double elapsed = std::clamp(
	    std::chrono::duration<double>(now - lastTick_).count(), 0.0, 0.25);
	lastTick_ = now;
	return elapsed;
}
} // namespace tanks::app
