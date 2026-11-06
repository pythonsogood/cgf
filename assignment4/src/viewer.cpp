#include "patch_scene.hpp"

namespace app = application;

namespace {
patch_scene::Patch patch;

void display() {
	app::clear();
	app::draw(patch.surface, app::flatShading, {0.75F, 0.45F, 0.25F});
	patch_scene::drawGuides(patch);
	glutSwapBuffers();
}

void showHelp() {
	app::showHelp();
	patch_scene::showHelp();
}

void keyboard(unsigned char key, int, int) {
	if (key == 'h' || key == 'H') {
		showHelp();
		return;
	}

	if (app::handleKey(key)) {
		return;
	}

	const auto edit = patch_scene::edit(patch, key);
	if (edit == patch_scene::Edit::None) {
		return;
	}
	
	if (edit == patch_scene::Edit::Geometry) {
		patch_scene::update(patch, true);
	}
	app::changed();
}

void cleanup() {
	patch_scene::cleanup(patch);
	app::cleanup();
}
} // namespace

int main(int argc, char **argv) {
	if (argc == 2 &&
	    (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")) {
		std::cout << "Usage: viewer [control-points.txt]" << std::endl;
		showHelp();
		return 0;
	}
	if (argc > 2) {
		std::cerr << "Usage: viewer [control-points.txt]" << std::endl;
		return 1;
	}
	int window = 0;
	try {
		if (argc == 2) {
			patch_scene::loadControlPoints(patch, argv[1]);
		}
		window = app::createWindow(argc, argv, "Bezier patch");
		glutCloseFunc(cleanup);
		patch_scene::update(patch, true);
		app::reset();
		showHelp();
		glutDisplayFunc(display);
		glutReshapeFunc(app::reshape);
		glutKeyboardFunc(keyboard);
		glutSpecialFunc(app::special);
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
