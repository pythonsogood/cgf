#pragma once
#include "app/input/input.hpp"
#include "app/cli/options.hpp"
#include "app/runtime/timing.hpp"
#include "app/platform/imgui_input.hpp"
#include "app/platform/window.hpp"
#include "game/world/world.hpp"
#include "graphics/renderer.hpp"
#include <exception>
#include <optional>

namespace tanks::app {
class Application {
	public:
	static int launch(int argc, char **argv);

	private:
	explicit Application(const Options &options);
	int run(char **argv);

	const Options &options_;
	World world_;
	std::optional<Window> window_;
	std::optional<Renderer> renderer_;
	std::optional<ImGuiInput> imguiInput_;
	FrameInfo frame_;
	InputController input_;
	LoopTiming timing_;
	int exitCode_ = 0;
	struct CameraInput {
		Vec2 orbit;
		float zoom = 0.0f;
	} cameraInput_;

	void registerCallbacks();
	static void displayCallback(void *user);
	static void reshapeCallback(int width, int height, void *user);
	static void keyboardCallback(unsigned char key, int x, int y, void *user);
	static void keyboardUpCallback(unsigned char key, int x, int y, void *user);
	static void specialCallback(int key, int x, int y, void *user);
	static void specialUpCallback(int key, int x, int y, void *user);
	static void mouseCallback(int button, int state, int x, int y, void *user);
	static void motionCallback(int x, int y, void *user);
	static void mouseWheelCallback(int button, int direction, int x, int y,
	                               void *user);
	static void entryCallback(int state, void *user);
	static void closeCallback(void *user);

	void deploy();
	void togglePause();
	void restart();
	PlayerInputs sampleInputs(bool captureKeyboard = false,
	                          bool captureMouse = false) const;
	void onKey(unsigned char key);
	void onSpecial(int key);
	void onResize(int width, int height);
	void onMotion(int x, int y);
	void onDisplay();
	void onClose();
	void quit(int code = 0);
	void fail(const std::exception &error);
};
} // namespace tanks::app
