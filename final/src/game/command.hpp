#pragma once
#include "core/math.hpp"
#include "game/input/keyboard.hpp"

namespace tanks {
enum class Controller { Human, Bot };
struct Command {
	Vec2 move;
	Vec2 aim;
	float turn = 0.0f;
	bool hasAim = false;
	bool fire = false;
};

struct PlayerActions {
	Vec2 move;
	Vec2 aim;
	bool hasAim = false;
	bool fire = false;
};

struct PlayerInputs {
	PlayerActions player;
	Keyboard keyboard;
	bool primaryDown = false;
	Vec2 screenRight{1.0f, 0.0f};
	Vec2 screenUp{0.0f, -1.0f};
};
} // namespace tanks
