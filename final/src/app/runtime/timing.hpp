#pragma once
#include <chrono>

namespace tanks::app {
class LoopTiming {
	public:
	void reset();
	double beginTick();
	double accumulator = 0.0;

	private:
	using Clock = std::chrono::steady_clock;
	Clock::time_point lastTick_ = Clock::now();
};
} // namespace tanks::app
