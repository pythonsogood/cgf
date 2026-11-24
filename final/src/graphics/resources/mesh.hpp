#pragma once
#include <GL/glew.h>

namespace tanks::graphics {
class Mesh {
	public:
	Mesh(GLuint vao, GLuint vbo, GLsizei count)
	    : vao_(vao), vbo_(vbo), count_(count) {}
	~Mesh();
	Mesh(const Mesh &) = delete;
	Mesh &operator=(const Mesh &) = delete;

	void draw() const;

	static Mesh cube();

	static Mesh cylinder();

	static Mesh arenaPrism();

	private:
	GLuint vao_ = 0, vbo_ = 0;
	GLsizei count_ = 0;
};
} // namespace tanks::graphics
