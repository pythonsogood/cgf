#include <GL/freeglut.h>
#include "app/input/input.hpp"
#include <optional>

namespace tanks::app {
namespace {
std::optional<Key> specialKey(int code) {
	if (code >= GLUT_KEY_F1 && code <= GLUT_KEY_F12)
		return static_cast<Key>(std::to_underlying(Key::F1) + (code - GLUT_KEY_F1));

	switch (code) {
	case GLUT_KEY_LEFT:
		return Key::Left;
	case GLUT_KEY_RIGHT:
		return Key::Right;
	case GLUT_KEY_UP:
		return Key::Up;
	case GLUT_KEY_DOWN:
		return Key::Down;
	case GLUT_KEY_HOME:
		return Key::Home;
	case GLUT_KEY_END:
		return Key::End;
	case GLUT_KEY_PAGE_UP:
		return Key::PageUp;
	case GLUT_KEY_PAGE_DOWN:
		return Key::PageDown;
	case GLUT_KEY_INSERT:
		return Key::Insert;
	case GLUT_KEY_DELETE:
		return Key::Delete;
	case GLUT_KEY_BEGIN:
		return Key::Begin;
	case GLUT_KEY_NUM_LOCK:
		return Key::NumLock;
	case GLUT_KEY_SHIFT_L:
		return Key::LeftShift;
	case GLUT_KEY_SHIFT_R:
		return Key::RightShift;
	case GLUT_KEY_CTRL_L:
		return Key::LeftCtrl;
	case GLUT_KEY_CTRL_R:
		return Key::RightCtrl;
	case GLUT_KEY_ALT_L:
		return Key::LeftAlt;
	case GLUT_KEY_ALT_R:
		return Key::RightAlt;
	case GLUT_KEY_SUPER_L:
		return Key::LeftSuper;
	case GLUT_KEY_SUPER_R:
		return Key::RightSuper;
	default:
		return std::nullopt;
	}
}
} // namespace
bool InputController::keyDown(unsigned char key) {
	if (keyboard_.isDown(key))
		return false;

	keyboard_.setDown(key, true);

	return true;
}

void InputController::keyUp(unsigned char key) {
	keyboard_.setDown(key, false);
}

std::optional<Key> InputController::specialDown(int key) {
	const auto mapped = specialKey(key);
	if (!mapped || keyboard_.isDown(*mapped))
		return std::nullopt;

	keyboard_.setDown(*mapped, true);

	return mapped;
}

void InputController::specialUp(int key) {
	if (const auto mapped = specialKey(key))
		keyboard_.setDown(*mapped, false);
}

void InputController::mouseButton(int button, int state, int x, int y) {
	pointer_.x = x;
	pointer_.y = y;

	if (button == GLUT_LEFT_BUTTON)
		pointer_.primaryDown = state == GLUT_DOWN;

	if (button == GLUT_RIGHT_BUTTON)
		pointer_.secondaryDown = state == GLUT_DOWN;
}

PointerMotion InputController::mouseMotion(int x, int y) {
	const PointerMotion motion{x - pointer_.x, y - pointer_.y,
	                           pointer_.secondaryDown};

	pointer_.x = x;
	pointer_.y = y;

	return motion;
}

void InputController::releaseAll() {
	keyboard_ = {};
	pointer_.primaryDown = false;
	pointer_.secondaryDown = false;
}

PlayerInputs InputController::sample(bool captureKeyboard,
                                     bool captureMouse) const {
	PlayerInputs result{};

	if (!captureKeyboard && !captureMouse)
		result.keyboard = keyboard_;

	if (!captureMouse)
		result.primaryDown = pointer_.primaryDown;

	return result;
}
} // namespace tanks::app
