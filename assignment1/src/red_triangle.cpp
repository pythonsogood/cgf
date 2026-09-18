#include "gl_utils.hpp"

void display() {
	gl_utils::beginFrame();
	glDrawArrays(GL_TRIANGLES, 0, 3);
	gl_utils::endFrame();
}

int main(int argc, char **argv) {
	const float vertices[] = {
	    -0.7F, -0.7F, 1.0F, 0.0F, 0.0F, 0.7F, -0.7F, 1.0F,
	    0.0F,  0.0F,  0.0F, 0.7F, 1.0F, 0.0F, 0.0F,
	};

	gl_utils::initialize(argc, argv, "Red Triangle", vertices, sizeof(vertices),
	                     false);
	glutDisplayFunc(display);
	glutMainLoop();
}
