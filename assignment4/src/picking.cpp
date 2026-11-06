#include "application.hpp"

#include <algorithm>
#include <array>
#include <filesystem>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

namespace app = application;

namespace {

using app::Geometry;
using app::Vec3;

struct Object {
	std::size_t geometry;
	Vec3 offset;
	float scale;
	std::array<float, 3> color;
};

struct Framebuffer {
	GLuint id = 0, color = 0, depth = 0;
};

std::vector<Geometry> geometries;
std::vector<Object> objects;
Framebuffer pickingBuffer{}, multisampleBuffer{};
int samples = 1;
bool antiAliasing = true;
std::mt19937 randomEngine{std::random_device{}()};

void destroyFramebuffer(Framebuffer &buffer) {
	glDeleteRenderbuffers(1, &buffer.color);
	glDeleteRenderbuffers(1, &buffer.depth);
	glDeleteFramebuffers(1, &buffer.id);

	buffer = {};
}

void resizeFramebuffer(Framebuffer &buffer, int sampleCount) {
	destroyFramebuffer(buffer);
	glGenFramebuffers(1, &buffer.id);
	glBindFramebuffer(GL_FRAMEBUFFER, buffer.id);
	glGenRenderbuffers(1, &buffer.color);
	glGenRenderbuffers(1, &buffer.depth);

	for (const bool color : {true, false}) {
		const GLuint renderbuffer = color ? buffer.color : buffer.depth;
		glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
		const GLenum format = color ? GL_RGBA8 : GL_DEPTH_COMPONENT24;
		if (sampleCount > 1) {
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, sampleCount, format,
			                                 app::width, app::height);
		} else {
			glRenderbufferStorage(GL_RENDERBUFFER, format, app::width, app::height);
		}
		glFramebufferRenderbuffer(GL_FRAMEBUFFER,
		                          color ? GL_COLOR_ATTACHMENT0 : GL_DEPTH_ATTACHMENT,
		                          GL_RENDERBUFFER, renderbuffer);
	}

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		throw std::runtime_error("Incomplete framebuffer");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void resizeFramebuffers() {
	resizeFramebuffer(pickingBuffer, 1);
	resizeFramebuffer(multisampleBuffer, samples);
}

void loadObjects(const std::vector<std::string> &paths) {
	geometries.resize(paths.size());

	for (std::size_t i = 0; i < paths.size(); ++i) {
		app::loadModel(geometries[i], paths[i]);
	}

	objects = {
	    {0, {-2.5F, 0, 0}, 0.9F, {0.9F, 0.15F, 0.12F}},
	    {paths.size() == 1 ? 0U : 1U, {0, 0, 0}, 0.9F, {0.12F, 0.65F, 0.3F}},
	    {paths.size() == 1 ? 0U : 2U, {2.5F, 0, 0}, 0.9F, {0.15F, 0.35F, 0.9F}}};

	app::sceneRadius = 3.4F;
}

void drawObjects(bool picking = false) {
	for (std::size_t i = 0; i < objects.size(); ++i) {
		const Object &object = objects[i];
		const std::array<float, 3> color =
		    picking ? std::array<float, 3>{static_cast<float>(i + 1) / 255.0F, 0, 0}
		            : object.color;

		app::draw(geometries[object.geometry],
		          picking ? app::unlit : app::solidPhong, color, object.offset,
		          object.scale);
	}
}

void showHelp() {
	app::showHelp();
	std::cout << "[Left click]: randomize the clicked object's diffuse color\n"
	             "[A]: toggle anti-aliasing"
	          << std::endl;
}

void display() {
	glBindFramebuffer(GL_FRAMEBUFFER, antiAliasing ? multisampleBuffer.id : 0);

	if (antiAliasing) {
		glEnable(GL_MULTISAMPLE);
	} else {
		glDisable(GL_MULTISAMPLE);
	}

	app::clear();
	drawObjects();

	if (antiAliasing) {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampleBuffer.id);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlitFramebuffer(0, 0, app::width, app::height, 0, 0, app::width,
		                  app::height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glutSwapBuffers();
}

void reshape(int width, int height) {
	const bool resized =
	    app::width != std::max(width, 1) || app::height != std::max(height, 1);

	app::reshape(width, height);

	if (resized || !pickingBuffer.id || !multisampleBuffer.id) {
		resizeFramebuffers();
	}
}

void keyboard(unsigned char key, int, int) {
	if (key == 'a' || key == 'A') {
		antiAliasing = !antiAliasing;
		glutPostRedisplay();
	} else if (key == 'h' || key == 'H') {
		showHelp();
	} else {
		app::handleKey(key);
	}
}

void mouse(int button, int state, int x, int y) {
	if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN || x < 0 ||
	    x >= app::width || y < 0 || y >= app::height || !pickingBuffer.id) {
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, pickingBuffer.id);
	glViewport(0, 0, app::width, app::height);

	glDisable(GL_MULTISAMPLE);
	glDisable(GL_DITHER);
	glDisable(GL_BLEND);
	glDisable(GL_FRAMEBUFFER_SRGB);

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	app::setView();

	drawObjects(true);

	std::array<GLubyte, 4> pixel{};
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glReadPixels(x, app::height - 1 - y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE,
	             pixel.data());
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (pixel[0] >= 1 && static_cast<std::size_t>(pixel[0]) <= objects.size()) {
		Object &object = objects[pixel[0] - 1];
		std::uniform_real_distribution<float> color(0.15F, 0.95F);

		for (float &channel : object.color) {
			channel = color(randomEngine);
		}

		glutPostRedisplay();
	}
}

void cleanup() {
	for (Geometry &geometry : geometries) {
		app::destroyGeometry(geometry);
	}

	geometries.clear();

	destroyFramebuffer(pickingBuffer);
	destroyFramebuffer(multisampleBuffer);

	app::cleanup();
}

void usage(const char *executable) {
	std::cout << "Usage: " << std::filesystem::path(executable).filename().string()
	          << " <model.smf> [second.smf third.smf]\n"
	             "       ";
	std::cout << std::filesystem::path(executable).filename().string()
	          << " --help\n";
}

} // namespace

int main(int argc, char **argv) {
	if (argc == 2 &&
	    (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")) {
		usage(argv[0]);
		showHelp();
		return 0;
	}

	if (argc != 2 && argc != 4) {
		usage(argv[0]);
		return 1;
	}

	int window = 0;

	try {
		window = app::createWindow(argc, argv, "Picking and anti-aliasing");

		glutCloseFunc(cleanup);

		GLint maximum = 0;
		glGetIntegerv(GL_MAX_SAMPLES, &maximum);

		samples = std::min(maximum, 4);

		if (samples < 2) {
			throw std::runtime_error("Multisample framebuffers are not supported");
		}

		loadObjects(std::vector<std::string>(argv + 1, argv + argc));

		app::reset();

		showHelp();

		resizeFramebuffers();

		glutDisplayFunc(display);
		glutReshapeFunc(reshape);
		glutKeyboardFunc(keyboard);
		glutSpecialFunc(app::special);
		glutMouseFunc(mouse);
		glutMainLoop();
	} catch (const std::exception &error) {
		std::cerr << error.what() << std::endl;

		if (window) {
			glutDestroyWindow(window);
		}

		return 1;
	}

	return 0;
}
