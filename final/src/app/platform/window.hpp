#pragma once

namespace tanks::app {
class Window {
	public:
	explicit Window(char **argv);
	~Window();
	Window(const Window &) = delete;
	Window &operator=(const Window &) = delete;
	void closed() { id_ = 0; }

	private:
	int id_ = 0;
};
} // namespace tanks::app
