#include "gl_utils.hpp"
#include "matrix_utils.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

enum Transformation { SCALE, ROTATE, TRANSLATE };
Transformation selected = ROTATE;
float scaling[3] = {1, 1, 1};
float rotation[3] = {-45, 35.264F, -30};
float translation[3] = {0, 0, 0};
float deltas[3] = {0.1F, 5.0F, 0.1F};

void quad(std::vector<float> &vertices, const float corners[][6], int a, int b,
          int c, int d) {
	const int indices[] = {a, b, c, a, c, d};

	for (int index : indices) {
		const float *corner = corners[index];

		vertices.insert(vertices.end(), corner, corner + 6);
	}
}

std::vector<float> buildCubeVertices() {
	const float corners[][6] = {
	    {-0.5F, -0.5F, 0.5F, 0, 0, 0},  {-0.5F, 0.5F, 0.5F, 1, 0, 0},
	    {0.5F, 0.5F, 0.5F, 1, 1, 0},    {0.5F, -0.5F, 0.5F, 0, 1, 0},
	    {-0.5F, -0.5F, -0.5F, 0, 0, 1}, {-0.5F, 0.5F, -0.5F, 1, 0, 1},
	    {0.5F, 0.5F, -0.5F, 1, 1, 1},   {0.5F, -0.5F, -0.5F, 0, 1, 1},
	};

	std::vector<float> vertices;
	vertices.reserve(36 * 6);

	quad(vertices, corners, 1, 0, 3, 2);
	quad(vertices, corners, 2, 3, 7, 6);
	quad(vertices, corners, 3, 0, 4, 7);
	quad(vertices, corners, 6, 5, 1, 2);
	quad(vertices, corners, 4, 5, 6, 7);
	quad(vertices, corners, 5, 4, 0, 1);

	return vertices;
}

// M = T * Rz * Ry * Rx * S
void transformationMatrix(matrix_utils::Matrix &transform) {
	matrix_utils::Matrix rx, ry, rz, r, s, t;

	matrix_utils::rotate(rx, rotation[0], 0);
	matrix_utils::rotate(ry, rotation[1], 1);
	matrix_utils::rotate(rz, rotation[2], 2);

	// Ry * Rx
	matrix_utils::multiply(r, ry, rx);
	// Rz * (Ry * Rx)
	matrix_utils::multiply(r, rz, r);

	matrix_utils::scale(s, scaling[0], scaling[1], scaling[2]);

	matrix_utils::translate(t, translation[0], translation[1], translation[2]);

	// R * S
	matrix_utils::multiply(transform, r, s);
	// T * (R * S)
	matrix_utils::multiply(transform, t, transform);
}

void display() {
	gl_utils::beginFrame();

	matrix_utils::Matrix transform;
	transformationMatrix(transform);
	gl_utils::setTransform(transform);

	glDrawArrays(GL_TRIANGLES, 0, 36);

	gl_utils::endFrame();
}

void menu(int action) { selected = static_cast<Transformation>(action); }

void reset() {
	for (int i = 0; i < 3; ++i) {
		scaling[i] = 1;
		rotation[i] = 0;
		translation[i] = 0;
	}

	deltas[SCALE] = 0.1F;
	deltas[ROTATE] = 5.0F;
	deltas[TRANSLATE] = 0.1F;
}

void keyboard(unsigned char key, int, int) {
	// ESC
	if (key == 27) {
		glutLeaveMainLoop();

		return;
	}

	if (key == '+' || key == '=') {
		deltas[selected] *= 2;

		return;
	}

	if (key == '-') {
		deltas[selected] = std::max(0.0001F, deltas[selected] / 2);

		return;
	}

	if (key == 'r') {
		reset();
		glutPostRedisplay();

		return;
	}

	int axis;
	float direction;

	switch (key) {
	case 'x':
		axis = 0;
		direction = 1;
		break;

	case 'X':
		axis = 0;
		direction = -1;
		break;

	case 'y':
		axis = 1;
		direction = 1;
		break;

	case 'Y':
		axis = 1;
		direction = -1;
		break;

	case 'z':
		axis = 2;
		direction = 1;
		break;

	case 'Z':
		axis = 2;
		direction = -1;
		break;

	default:
		return;
	}

	float delta = direction * deltas[selected];

	if (selected == SCALE) {
		scaling[axis] += delta;
	} else if (selected == ROTATE) {
		rotation[axis] += delta;
	} else if (selected == TRANSLATE) {
		translation[axis] += delta;
	}

	glutPostRedisplay();
}

int main(int argc, char **argv) {
	std::cout << "[RMB] transformation selector menu (ROTATE by default)\n"
	             "[+ / =] double the selected transformation's delta\n"
	             "[-] halve the selected transformation's delta\n"
	             "[x / y / z] increase X / Y / Z\n"
	             "[X / Y / Z] decrease X / Y / Z\n"
	             "[r] reset all transformations and initial deltas.\n"
	             "[ESC] close"
	          << std::endl;

	const std::vector<float> vertices = buildCubeVertices();

	gl_utils::initialize(argc, argv, "Colored Cube", vertices.data(),
	                     vertices.size() * sizeof(float), 3, true);

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutCloseFunc(gl_utils::cleanup);

	glutCreateMenu(menu);
	glutAddMenuEntry("SCALE", SCALE);
	glutAddMenuEntry("ROTATE", ROTATE);
	glutAddMenuEntry("TRANSLATE", TRANSLATE);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	glutMainLoop();

	return 0;
}
