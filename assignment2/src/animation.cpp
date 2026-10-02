#include "gl_utils.hpp"
#include "matrix_utils.hpp"

#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

constexpr int SEGMENTS = 80;
constexpr int CIRCLE_COUNT = SEGMENTS + 2;
constexpr int SQUARE_COUNT = 6;
constexpr int TRIANGLE_START = CIRCLE_COUNT;
constexpr int SQUARES_START = TRIANGLE_START + 3;

int mainWindow, ellipseWindow, secondWindow;
int lastTime;
float animationTime = 0.0;
bool animating = true;
float squareColor[3] = {1.0F, 1.0F, 1.0F};
float shapeColor[3] = {1.0F, 0.0F, 0.0F};

struct Circle {
	float x, y;
	float color[3];
};
std::vector<Circle> circles;
std::mt19937 randomGenerator{std::random_device{}()};
std::uniform_real_distribution<float> random_color(0.25F, 1.0F);

void randomColor(float color[3]) {
	color[0] = random_color(randomGenerator);
	color[1] = random_color(randomGenerator);
	color[2] = random_color(randomGenerator);
}

void changeColor(float color[3], float r, float g, float b) {
	color[0] = r;
	color[1] = g;
	color[2] = b;
}

std::vector<float> buildVertices() {
	std::vector<float> vertices;

	vertices.insert(vertices.end(), {0, 0, 1, 1, 1});
	for (int i = 0; i <= SEGMENTS; ++i) {
		float angle = 2.0F * M_PI * i / SEGMENTS;
		vertices.insert(vertices.end(), {std::cos(angle), std::sin(angle), 1, 1, 1});
	}

	vertices.insert(vertices.end(), {0, 0.36F, 1, 1, 1});
	vertices.insert(vertices.end(), {-0.32F, -0.18F, 1, 1, 1});
	vertices.insert(vertices.end(), {0.32F, -0.18F, 1, 1, 1});
	const float sizes[] = {0.55F, 0.45F, 0.35F, 0.25F, 0.15F, 0.07F};
	for (int i = 0; i < SQUARE_COUNT; ++i) {
		float s = sizes[i];
		float color = i % 2 == 0 ? 1.0F : 0.0F;
		vertices.insert(vertices.end(), {-s, s, color, color, color});
		vertices.insert(vertices.end(), {-s, -s, color, color, color});
		vertices.insert(vertices.end(), {s, s, color, color, color});
		vertices.insert(vertices.end(), {s, -s, color, color, color});
	}
	return vertices;
}

float rotationAngle() { return std::fmod(animationTime * 45.0, 360.0); }

float breathingScale(float base = 1.0f, float scale = 0.2f,
                     float speed = 2.0f) {
	return base + scale * std::sin(animationTime * speed);
}

void displayMain() {
	gl_utils::beginFrame();

	matrix_utils::Matrix transform;
	matrix_utils::rotate(transform, rotationAngle(), 2);
	gl_utils::setTransform(transform);

	for (int i = 0; i < SQUARE_COUNT; ++i) {
		if (i % 2 == 0) {
			gl_utils::setSolidColor(squareColor[0], squareColor[1], squareColor[2]);
		} else {
			gl_utils::setSolidColor(0, 0, 0);
		}

		glDrawArrays(GL_TRIANGLE_STRIP, SQUARES_START + i * 4, 4);
	}

	for (const Circle &circle : circles) {
		float radius = 0.1f * breathingScale();

		matrix_utils::Matrix translation, scaling;
		matrix_utils::translate(translation, circle.x, circle.y);
		matrix_utils::scale(scaling, radius, radius);
		matrix_utils::multiply(transform, translation, scaling);
		gl_utils::setTransform(transform);

		gl_utils::setSolidColor(circle.color[0], circle.color[1], circle.color[2]);

		glDrawArrays(GL_TRIANGLE_FAN, 0, CIRCLE_COUNT);
	}

	gl_utils::endFrame();
}

void displayEllipse() {
	gl_utils::beginFrame();

	matrix_utils::Matrix transform;
	matrix_utils::scale(transform, 0.75F, 0.45F);
	gl_utils::setTransform(transform);
	gl_utils::setSolidColor(1, 0, 0);

	glDrawArrays(GL_TRIANGLE_FAN, 0, CIRCLE_COUNT);

	gl_utils::endFrame();
}

void displaySecond() {
	gl_utils::beginFrame();

	gl_utils::setSolidColor(shapeColor[0], shapeColor[1], shapeColor[2]);
	matrix_utils::Matrix transform, translation, rotation, scaling;
	matrix_utils::translate(translation, -0.45F, 0);
	matrix_utils::rotate(rotation, -rotationAngle(), 2);
	matrix_utils::multiply(transform, translation, rotation);
	gl_utils::setTransform(transform);

	glDrawArrays(GL_TRIANGLES, TRIANGLE_START, 3);

	float radius = 0.27f * breathingScale();

	matrix_utils::translate(translation, 0.45F, 0);
	matrix_utils::scale(scaling, radius, radius);
	matrix_utils::multiply(transform, translation, scaling);
	gl_utils::setTransform(transform);

	glDrawArrays(GL_TRIANGLE_FAN, 0, CIRCLE_COUNT);

	gl_utils::endFrame();
}

void redisplayAnimatedWindows() {
	int current = glutGetWindow();

	glutSetWindow(mainWindow);
	glutPostRedisplay();

	glutSetWindow(secondWindow);
	glutPostRedisplay();

	glutSetWindow(current);
}

void idle() {
	int now = glutGet(GLUT_ELAPSED_TIME);
	animationTime += (now - lastTime) / 1000.0;
	lastTime = now;

	redisplayAnimatedWindows();
}

enum MainAction { STOP = 1, START, WHITE, RED, GREEN };

void mainMenu(int action) {
	if (action == STOP) {
		if (animating) {
			idle();
		}

		animating = false;
		glutIdleFunc(nullptr);
	} else if (action == START) {
		animating = true;
		lastTime = glutGet(GLUT_ELAPSED_TIME);
		glutIdleFunc(idle);
	} else {
		switch (action) {
		case WHITE:
			changeColor(squareColor, 1, 1, 1);
			break;
		case RED:
			changeColor(squareColor, 1, 0, 0);
			break;
		case GREEN:
			changeColor(squareColor, 0, 1, 0);
			break;
		default:
			return;
		}
	}

	redisplayAnimatedWindows();
}

enum EllipseBackgroundColor {
	ELLIPSE_BACKGROUND_DARK_BLUE,
	ELLIPSE_BACKGROUND_BLUE,
	ELLIPSE_BACKGROUND_GREEN,
	ELLIPSE_BACKGROUND_WHITE
};

void ellipseMenu(int action) {
	switch (action) {
	case ELLIPSE_BACKGROUND_DARK_BLUE:
		glClearColor(0.12F, 0.18F, 0.28F, 1);
		break;
	case ELLIPSE_BACKGROUND_BLUE:
		glClearColor(0.0F, 0.35F, 0.65F, 1);
		break;
	case ELLIPSE_BACKGROUND_GREEN:
		glClearColor(0.2F, 0.5F, 0.2F, 1);
		break;
	case ELLIPSE_BACKGROUND_WHITE:
		glClearColor(1, 1, 1, 1);
		break;
	default:
		return;
	}

	glutPostRedisplay();
}

void keyboard(unsigned char key, int, int) {
	if (key == 27) {
		glutLeaveMainLoop();
		return;
	}

	if (glutGetWindow() != secondWindow) {
		return;
	}

	switch (key) {
	case 'r':
		changeColor(shapeColor, 1, 0, 0);
		break;
	case 'g':
		changeColor(shapeColor, 0, 1, 0);
		break;
	case 'b':
		changeColor(shapeColor, 0, 0, 1);
		break;
	case 'y':
		changeColor(shapeColor, 1, 1, 0);
		break;
	case 'o':
		changeColor(shapeColor, 1, 0.5F, 0);
		break;
	case 'p':
		changeColor(shapeColor, 0.5F, 0, 0.5F);
		break;
	case 'w':
		changeColor(shapeColor, 1, 1, 1);
		break;
	default:
		return;
	}

	glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
	if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN) {
		return;
	}

	float width = static_cast<float>(glutGet(GLUT_WINDOW_WIDTH));
	float height = static_cast<float>(glutGet(GLUT_WINDOW_HEIGHT));

	Circle circle{2.0F * x / width - 1.0F, 1.0F - 2.0F * y / height};
	randomColor(circle.color);

	circles.push_back(circle);

	glutPostRedisplay();
}

void resizeMain(int width, int height) {
	gl_utils::resize(width, height);

	if (ellipseWindow == 0) {
		return;
	}

	glutSetWindow(ellipseWindow);
	glutPositionWindow(0, 0);
	glutReshapeWindow(std::max(1, width / 3), std::max(1, height / 3));
	glutSetWindow(mainWindow);
}

int main(int argc, char **argv) {
	const std::vector<float> vertices = buildVertices();

	gl_utils::initialize(argc, argv, "black & white", vertices.data(),
	                     vertices.size() * sizeof(float));

	mainWindow = glutGetWindow();

	glutPositionWindow(60, 80);

	glutDisplayFunc(displayMain);
	glutReshapeFunc(resizeMain);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutCloseFunc(gl_utils::cleanup);

	int colors = glutCreateMenu(mainMenu);
	glutAddMenuEntry("White", WHITE);
	glutAddMenuEntry("Red", RED);
	glutAddMenuEntry("Green", GREEN);
	glutCreateMenu(mainMenu);
	glutAddMenuEntry("Stop Animation", STOP);
	glutAddMenuEntry("Start Animation", START);
	glutAddSubMenu("Square Colors", colors);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	ellipseWindow = glutCreateSubWindow(mainWindow, 0, 0, 200, 200);
	gl_utils::initializeContext(vertices.data(), vertices.size() * sizeof(float));
	glClearColor(0.12F, 0.18F, 0.28F, 1);
	glutDisplayFunc(displayEllipse);
	glutKeyboardFunc(keyboard);
	glutCloseFunc(gl_utils::cleanup);
	glutCreateMenu(ellipseMenu);
	glutAddMenuEntry("Dark Blue", ELLIPSE_BACKGROUND_DARK_BLUE);
	glutAddMenuEntry("Blue", ELLIPSE_BACKGROUND_BLUE);
	glutAddMenuEntry("Green", ELLIPSE_BACKGROUND_GREEN);
	glutAddMenuEntry("White", ELLIPSE_BACKGROUND_WHITE);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	secondWindow = glutCreateWindow("window 2");
	gl_utils::initializeContext(vertices.data(), vertices.size() * sizeof(float));
	glutPositionWindow(700, 80);
	glutDisplayFunc(displaySecond);
	glutKeyboardFunc(keyboard);
	glutCloseFunc(gl_utils::cleanup);

	lastTime = glutGet(GLUT_ELAPSED_TIME);
	glutIdleFunc(idle);

	glutMainLoop();

	return 0;
}
