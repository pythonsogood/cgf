#include "patch_scene.hpp"
#include "texture_image.hpp"

namespace app = application;

namespace {
patch_scene::Patch patch;
GLuint image = 0;

void initializeTexture() {
	constexpr int size = 256;
	const auto pixels = texture::generate(size);

	glGenTextures(1, &image);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, image);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, size, size, 0, GL_RGB,
	             GL_UNSIGNED_BYTE, pixels.data());

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void display() {
	app::clear();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, image);

	app::draw(patch.surface, app::imagePhong, {0.75F, 0.45F, 0.25F});
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
		patch_scene::update(patch, false);
	}

	app::changed();
}

void cleanup() {
	glDeleteTextures(1, &image);
	image = 0;
	patch_scene::cleanup(patch);
	app::cleanup();
}
} // namespace

int main(int argc, char **argv) {
	if (argc == 2 &&
	    (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")) {
		std::cout << "Usage: texture2d [control-points.txt]" << std::endl;
		showHelp();

		return 0;
	}

	if (argc > 2) {
		std::cerr << "Usage: texture2d [control-points.txt]" << std::endl;
		return 1;
	}

	patch.resolution = 12;
	int window = 0;
	try {
		if (argc == 2) {
			patch_scene::loadControlPoints(patch, argv[1]);
		}

		window = app::createWindow(argc, argv, "Image texture");
		glutCloseFunc(cleanup);

		initializeTexture();
		patch_scene::update(patch, false);

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
