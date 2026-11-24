#pragma once

namespace tanks::app {
class ImGuiInput {
	public:
	ImGuiInput();
	~ImGuiInput();
	ImGuiInput(const ImGuiInput &) = delete;
	ImGuiInput &operator=(const ImGuiInput &) = delete;

	void beginFrame();
	void keyboard(unsigned char key, int x, int y);
	void keyboardUp(unsigned char key, int x, int y);
	void special(int key, int x, int y);
	void specialUp(int key, int x, int y);
	void mouseButton(int button, int state, int x, int y);
	void mouseMotion(int x, int y);
	void mouseWheel(int button, int direction, int x, int y);
	void resize(int width, int height);
	bool captureMouse() const;
	bool captureKeyboard() const;
};
} // namespace tanks::app
