#include "application.hpp"

#include <exception>
#include <iostream>
#include <string>

namespace app = application;

namespace {

app::Geometry model;

void display() {
	app::clear();

	app::draw(model, app::cloudPhong, {0.9F, 0.1F, 0.1F});

	glutSwapBuffers();
}

void keyboard(unsigned char key, int, int) {
	if (key == 'h' || key == 'H') {
		app::showHelp();
		return;
	}

	app::handleKey(key);
}

void cleanup() {
	app::destroyGeometry(model);
	app::cleanup();
}

} // namespace

int main(int argc, char **argv) {
	const char *usage = "Usage: texture3d model.smf";
	if (argc == 2 &&
	    (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")) {
		std::cout << usage << '\n';
		app::showHelp();
		return 0;
	}

	if (argc != 2) {
		std::cerr << usage << '\n';
		return 1;
	}

	int window = 0;
	try {
		window = app::createWindow(argc, argv, "3D procedural texture");

		glutCloseFunc(cleanup);

		app::loadModel(model, argv[1]);

		app::reset();

		app::showHelp();

		glutDisplayFunc(display);
		glutKeyboardFunc(keyboard);
		glutReshapeFunc(app::reshape);
		glutSpecialFunc(app::special);
		glutMainLoop();
	} catch (const std::exception &error) {
		std::cerr << error.what() << '\n';
		if (window != 0) {
			glutDestroyWindow(window);
		}
		
		return 1;
	}
	return 0;
}
