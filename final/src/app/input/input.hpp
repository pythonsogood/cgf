#pragma once
#include "game/command.hpp"
#include <optional>

namespace tanks::app {
struct PointerState {
	int x = 640, y = 360;
	bool primaryDown = false;
	bool secondaryDown = false;
};

struct PointerMotion {
	int x = 0, y = 0;
	bool secondaryDown = false;
};

class InputController {
	public:
	bool keyDown(unsigned char key);
	void keyUp(unsigned char key);
	std::optional<Key> specialDown(int key);
	void specialUp(int key);
	void mouseButton(int button, int state, int x, int y);
	PointerMotion mouseMotion(int x, int y);
	void releaseAll();
	PlayerInputs sample(bool captureKeyboard = false,
	                    bool captureMouse = false) const;

	private:
	Keyboard keyboard_;
	PointerState pointer_;
};
} // namespace tanks::app
