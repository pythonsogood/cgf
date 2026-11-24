#include <GL/glew.h>
#include <GL/freeglut.h>
#include "app/runtime/application.hpp"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <stdexcept>

namespace tanks::app {
namespace {
void checkRendering() {
	GLenum error;
	bool failed = false;

	while ((error = glGetError()) != GL_NO_ERROR) {
		std::cerr << "OpenGL error after frame: 0x" << std::hex << error << std::dec
		          << '\n';
		failed = true;
	}

	if (failed)
		throw std::runtime_error("Rendering failed (see OpenGL diagnostics)");
}
} // namespace

Application::Application(const Options &options)
    : options_(options), world_(freeForAllConfig()) {}

int Application::run(char **argv) {
	window_.emplace(argv);
	renderer_.emplace(options_.shaders.string());
	imguiInput_.emplace();

	onResize(1280, 720);

	registerCallbacks();

	glutIgnoreKeyRepeat(1);

	timing_.reset();

	glutMainLoop();

	return exitCode_;
}

void Application::deploy() {
	if (world_.phase() == Phase::Ready)
		world_.start();
	else if (world_.phase() == Phase::MatchOver)
		world_.restart();
}

void Application::togglePause() {
	frame_.paused = !frame_.paused;
	timing_.accumulator = 0;
}

void Application::restart() {
	world_.restart();

	frame_.paused = false;
	timing_.accumulator = 0;
}

void Application::onKey(unsigned char key) {
	if (!input_.keyDown(key) || (imguiInput_->captureKeyboard() && key != 27))
		return;

	switch (std::tolower(key)) {
	case 27:
		quit();
		break;
	case '\r':
		deploy();
		break;
	case 'p':
		togglePause();
		break;
	case 'r':
		restart();
		break;
	case 'c':
		renderer_->resetCamera();
		frame_.orthographic = true;
		break;
	case 'o':
		frame_.orthographic = !frame_.orthographic;
		break;
	}
}

void Application::onSpecial(int key) {
	const auto pressed = input_.specialDown(key);
	if (!pressed)
		return;

	if (*pressed == Key::F1)
		printControls(std::cout);

	if (imguiInput_->captureKeyboard() || imguiInput_->captureMouse())
		return;

	auto &light = world_.light();
	light.input.keyPressed(*pressed);

	if (*pressed == Key::F3 && light.active())
		std::cout << "Technique (shadow mapping): "
		          << (light.castsShadows ? "ON" : "OFF") << '\n';
}

void Application::onResize(int width, int height) {
	frame_.width = std::max(1, width);
	frame_.height = std::max(1, height);

	if (imguiInput_)
		imguiInput_->resize(frame_.width, frame_.height);

	if (renderer_)
		renderer_->resize(frame_.width, frame_.height);
}

void Application::onMotion(int x, int y) {
	const auto motion = input_.mouseMotion(x, y);

	if (motion.secondaryDown)
		cameraInput_.orbit += {float(motion.x), float(motion.y)};
}

PlayerInputs Application::sampleInputs(bool captureKeyboard,
                                       bool captureMouse) const {
	PlayerInputs inputs = input_.sample(captureKeyboard, captureMouse);

	inputs.screenRight = renderer_->screenRight();
	inputs.screenUp = {inputs.screenRight.z, -inputs.screenRight.x};

	return inputs;
}

void Application::onDisplay() {
	if (!renderer_)
		return;

	try {
		imguiInput_->beginFrame();

		if (!imguiInput_->captureMouse()) {
			if (length(cameraInput_.orbit) > 0)
				renderer_->orbit(cameraInput_.orbit.x, cameraInput_.orbit.z);

			if (cameraInput_.zoom != 0)
				renderer_->zoom(cameraInput_.zoom);
		}

		cameraInput_ = {};
		const double elapsed = timing_.beginTick();
		const auto inputs =
		    sampleInputs(imguiInput_->captureKeyboard(), imguiInput_->captureMouse());

		tanks::updateControls(world_.light(), inputs, static_cast<float>(elapsed),
		                      world_);

		if (!frame_.paused)
			timing_.accumulator += elapsed;

		while (timing_.accumulator >= fixedStep) {
			world_.step(fixedStep, inputs);
			timing_.accumulator -= fixedStep;
		}

		renderer_->draw(world_, frame_);

		const auto actions = renderer_->takeHudActions();

		if (actions.deploy)
			deploy();

		if (actions.restart)
			restart();

		if (actions.togglePause)
			togglePause();

		checkRendering();

		glutSwapBuffers();
	} catch (const std::exception &error) {
		fail(error);
	}
}

void Application::onClose() {
	imguiInput_.reset();
	renderer_.reset();

	if (window_)
		window_->closed();
}

void Application::quit(int code) {
	exitCode_ = code;

	imguiInput_.reset();
	renderer_.reset();

	glutLeaveMainLoop();
}

void Application::fail(const std::exception &error) {
	std::cerr << "Application error: " << error.what() << '\n';
	quit(1);
}

int Application::launch(int argc, char **argv) {
	try {
		std::cout << std::unitbuf;
		const Options options = parseOptions(argc, argv);

		if (options.help) {
			printUsage(std::cout);
			return 0;
		}

		printControls(std::cout);

		Application application(options);
		return application.run(argv);
	} catch (const std::exception &error) {
		std::cerr << "Startup error: " << error.what() << '\n';

		return 1;
	}
}
} // namespace tanks::app
