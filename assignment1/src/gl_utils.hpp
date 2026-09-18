#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <cstddef>
#include <cstdio>

namespace gl_utils {

inline GLuint program;
inline GLuint vao;
inline bool debugMode;
inline const float *debugVertices;
inline int debugVertexCount;

inline const char *vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec2 vPosition;
layout(location = 1) in vec3 vColor;
out vec3 color;

void main() {
    gl_Position = vec4(vPosition, 0.0, 1.0);
    color = vColor;
}
)";

inline const char *fragmentShaderSource = R"(
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

inline void drawDebugText() {
	if (!debugMode) {
		return;
	}

	glUseProgram(0);
	glColor3f(1.0F, 1.0F, 1.0F);

	for (int i = 0; i < debugVertexCount; ++i) {
		float x = debugVertices[i * 5];
		float y = debugVertices[i * 5 + 1];

		char text[64];
		std::snprintf(text, sizeof(text), "v%d (%.2f, %.2f)", i, x, y);

		glRasterPos2f(x + 0.02F, y + 0.02F);
		for (const char *c = text; *c; ++c) {
			glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *c);
		}
	}
}

inline void initialize(int &argc, char **argv, const char *title,
                       const float *vertices, std::size_t size,
                       bool debug = false) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(debug ? GLUT_COMPATIBILITY_PROFILE : GLUT_CORE_PROFILE);
	glutInitWindowSize(500, 500);
	glutCreateWindow(title);

	debugMode = debug;
	debugVertices = vertices;
	debugVertexCount = static_cast<int>(size / (5 * sizeof(float)));

	glewExperimental = GL_TRUE;
	glewInit();

	GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
	GLuint fragmentShader =
	    compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
	program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	GLuint vbo;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
	                      reinterpret_cast<void *>(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
	glutReshapeFunc(resize);
}

inline void beginFrame() {
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(program);
	glBindVertexArray(vao);
}

inline void endFrame() {
	drawDebugText();
	glutSwapBuffers();
}

} // namespace gl_utils
