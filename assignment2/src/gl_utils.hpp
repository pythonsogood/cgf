#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>

#include "matrix_utils.hpp"

#include <unordered_map>

namespace gl_utils {

struct Window {
	GLuint program = 0, vao = 0, vbo = 0;
	GLint transform = -1, isSolid = -1, solidColor = -1;
	bool depth = false;
};

inline std::unordered_map<int, Window> windows;

inline const char *vertex_shader_source = R"(
#version 330 core
layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vColor;
uniform mat4 transform;
uniform bool isSolid;
uniform vec3 solidColor;
out vec3 color;

void main() {
    gl_Position = transform * vec4(vPosition, 1.0);
    color = isSolid ? solidColor : vColor;
}
)";

inline const char *fragment_shader_source = R"(
#version 330 core
in vec3 color;
out vec4 fragmentColor;

void main() {
    fragmentColor = vec4(color, 1.0);
}
)";

inline GLuint compileShader(GLenum type, const char *source) {
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, nullptr);
	glCompileShader(shader);
	return shader;
}

inline void resize(int width, int height) { glViewport(0, 0, width, height); }

inline void initializeContext(const float *vertices, std::size_t size,
                              int positionComponents = 2, bool depth = false) {
	glewExperimental = GL_TRUE;
	glewInit();

	Window window;
	window.depth = depth;

	GLuint vertex_shader = compileShader(GL_VERTEX_SHADER, vertex_shader_source);
	GLuint fragment_shader =
	    compileShader(GL_FRAGMENT_SHADER, fragment_shader_source);
	window.program = glCreateProgram();
	glAttachShader(window.program, vertex_shader);
	glAttachShader(window.program, fragment_shader);
	glLinkProgram(window.program);
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	window.transform = glGetUniformLocation(window.program, "transform");
	window.isSolid = glGetUniformLocation(window.program, "isSolid");
	window.solidColor = glGetUniformLocation(window.program, "solidColor");

	glGenVertexArrays(1, &window.vao);
	glGenBuffers(1, &window.vbo);
	glBindVertexArray(window.vao);
	glBindBuffer(GL_ARRAY_BUFFER, window.vbo);
	glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(size), vertices,
	             GL_STATIC_DRAW);

	GLsizei bytesPerVertex = (positionComponents + 3) * sizeof(float);
	glVertexAttribPointer(0, positionComponents, GL_FLOAT, GL_FALSE,
	                      bytesPerVertex, nullptr);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
	    1, 3, GL_FLOAT, GL_FALSE, bytesPerVertex,
	    reinterpret_cast<void *>(positionComponents * sizeof(float)));
	glEnableVertexAttribArray(1);

	windows.emplace(glutGetWindow(), window);

	if (depth) {
		glEnable(GL_DEPTH_TEST);
	}

	glClearColor(0.0F, 0.0F, 0.0F, 1.0F);

	glutReshapeFunc(resize);
}

inline void initialize(int &argc, char **argv, const char *title,
                       const float *vertices, std::size_t size,
                       int positionComponents = 2, bool depth = false) {
	glutInit(&argc, argv);

	unsigned int display_mode = GLUT_DOUBLE | GLUT_RGBA;
	if (depth) {
		display_mode |= GLUT_DEPTH;
	}

	glutInitDisplayMode(display_mode);

	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	glutInitWindowSize(600, 600);
	glutCreateWindow(title);

	initializeContext(vertices, size, positionComponents, depth);
}

inline void setTransform(const matrix_utils::Matrix &transform) {
	glUniformMatrix4fv(windows.at(glutGetWindow()).transform, 1, GL_TRUE,
	                   transform);
}

inline void setSolidColor(float r, float g, float b) {
	const Window &window = windows.at(glutGetWindow());

	glUniform1i(window.isSolid, GL_TRUE);
	glUniform3f(window.solidColor, r, g, b);
}

inline void beginFrame() {
	const Window &window = windows.at(glutGetWindow());

	GLbitfield clearMask = GL_COLOR_BUFFER_BIT;
	if (window.depth) {
		clearMask |= GL_DEPTH_BUFFER_BIT;
	}

	glClear(clearMask);
	glUseProgram(window.program);
	glBindVertexArray(window.vao);

	matrix_utils::Matrix transform;
	matrix_utils::identity(transform);
	setTransform(transform);

	glUniform1i(window.isSolid, GL_FALSE);
}

inline void endFrame() { glutSwapBuffers(); }

inline void cleanup() {
	int window = glutGetWindow();
	auto found = windows.find(window);

	if (found == windows.end()) {
		return;
	}

	glDeleteBuffers(1, &found->second.vbo);
	glDeleteVertexArrays(1, &found->second.vao);
	glDeleteProgram(found->second.program);

	windows.erase(found);
}

} // namespace gl_utils
