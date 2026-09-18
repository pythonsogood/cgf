#include "gl_utils.hpp"

#include <cmath>
#include <vector>
#include <math.h>

constexpr int SEGMENTS = 40;
constexpr int ELLIPSE_COUNT = SEGMENTS + 2;
constexpr int TRIANGLE_START = ELLIPSE_COUNT;
constexpr int CIRCLE_START = TRIANGLE_START + 3;
constexpr int CIRCLE_COUNT = SEGMENTS + 2;
constexpr int SQUARES_START = CIRCLE_START + CIRCLE_COUNT;

void addVertex(std::vector<float> &vertices, float x, float y, float r, float g,
               float b) {
	vertices.insert(vertices.end(), {x, y, r, g, b});
}

void display() {
	gl_utils::beginFrame();

	glDrawArrays(GL_TRIANGLE_FAN, 0, ELLIPSE_COUNT);
	glDrawArrays(GL_TRIANGLES, TRIANGLE_START, 3);
	glDrawArrays(GL_TRIANGLE_FAN, CIRCLE_START, CIRCLE_COUNT);
	for (int i = 0; i < 6; ++i) {
		glDrawArrays(GL_TRIANGLE_STRIP, SQUARES_START + i * 4, 4);
	}

	gl_utils::endFrame();
}

int main(int argc, char **argv) {
	std::vector<float> vertices;

	// ellipse
	addVertex(vertices, -0.6F, 0.6F, 1.0F, 0.0F, 0.0F);
	for (int i = 0; i <= SEGMENTS; ++i) {
		float angle = 2.0F * M_PI * i / SEGMENTS;
		addVertex(vertices, -0.6F + 0.22F * std::cos(angle),
		          0.6F + 0.132F * std::sin(angle), 1.0F, 0.0F, 0.0F);
	}

	// triangle
	addVertex(vertices, 0.0F, 0.9F, 1.0F, 0.0F, 0.0F);
	addVertex(vertices, -0.3F, 0.35F, 0.0F, 1.0F, 0.0F);
	addVertex(vertices, 0.3F, 0.35F, 0.0F, 0.0F, 1.0F);

	// circle
	addVertex(vertices, 0.6F, 0.6F, 0.5F, 0.0F, 0.0F);
	for (int i = 0; i <= SEGMENTS; ++i) {
		float angle = 2.0F * M_PI * i / SEGMENTS;
		float red = (1.0F - std::cos(angle)) / 2.0F;
		addVertex(vertices, 0.6F + 0.2F * std::cos(angle),
		          0.6F + 0.2F * std::sin(angle), red, 0.0F, 0.0F);
	}

	// squares
	const float sizes[] = {0.55F, 0.45F, 0.35F, 0.25F, 0.15F, 0.07F};
	for (int i = 0; i < 6; ++i) {
		float s = sizes[i];
		float color = i % 2 == 0 ? 1.0F : 0.0F;
		addVertex(vertices, -s, -0.35F + s, color, color, color);
		addVertex(vertices, -s, -0.35F - s, color, color, color);
		addVertex(vertices, s, -0.35F + s, color, color, color);
		addVertex(vertices, s, -0.35F - s, color, color, color);
	}

	gl_utils::initialize(argc, argv, "2D Polygons", vertices.data(),
	                     vertices.size() * sizeof(float), false);
	glutDisplayFunc(display);
	glutMainLoop();
}
