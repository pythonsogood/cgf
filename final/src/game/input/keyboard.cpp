#include "game/input/keyboard.hpp"
#include <cctype>

namespace tanks {
Key Keyboard::character(unsigned char key) {
	return static_cast<Key>(std::tolower(key));
}
} // namespace tanks
