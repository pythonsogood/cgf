#pragma once
#include <bitset>
#include <utility>

namespace tanks {
enum class Key : unsigned {
	Backspace = 8,
	Tab = 9,
	Enter = 13,
	Escape = 27,
	Space = 32,
	Delete = 127,
	Left = 256,
	Right,
	Up,
	Down,
	Home,
	End,
	PageUp,
	PageDown,
	Insert,
	Begin,
	NumLock,
	LeftShift,
	RightShift,
	LeftCtrl,
	RightCtrl,
	LeftAlt,
	RightAlt,
	LeftSuper,
	RightSuper,
	F1,
	F2,
	F3,
	F4,
	F5,
	F6,
	F7,
	F8,
	F9,
	F10,
	F11,
	F12,
	Count
};

class Keyboard {
	public:
	bool isDown(Key key) const { return held_.test(std::to_underlying(key)); }
	bool isDown(unsigned char key) const { return isDown(character(key)); }

	void setDown(Key key, bool down) { held_.set(std::to_underlying(key), down); }
	void setDown(unsigned char key, bool down) { setDown(character(key), down); }

	private:
	static Key character(unsigned char key);
	std::bitset<std::to_underlying(Key::Count)> held_;
};
} // namespace tanks
