#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>

#include "bezier_patch.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

namespace application {

using Matrix = float[16];
using bezier::Vertex;
using smf::Vec3;
inline constexpr float halfFov = M_PI / 8.0F;

inline constexpr int flatShading = 0;
inline constexpr int solidPhong = 1;
inline constexpr int imagePhong = 2;
inline constexpr int cloudPhong = 3;
inline constexpr int unlit = 4;

struct Cylinder {
	float angle, radius, height;

	Vec3 position() const {
		return {radius * std::cos(angle), height, radius * std::sin(angle)};
	}
};

struct Geometry {
	GLuint vao = 0, vbo = 0;
	GLsizei count = 0;
};

struct Uniforms {
	GLint model, view, projection, renderMode, diffuseColor;
};

inline Uniforms uniforms{};
inline GLuint program = 0;
inline Cylinder camera{};
inline Vec3 target{};
inline float sceneRadius = 1;
inline int width = 1000, height = 800;

inline void reset() {
	camera = {static_cast<float>(M_PI / 2), sceneRadius * 3.3F,
	          sceneRadius * 0.55F};
}

inline void lookAt(Matrix &result, Vec3 eye, Vec3 center) {
	const Vec3 forward = smf::normalized(center - eye);
	const Vec3 right = smf::normalized(smf::cross(forward, {0, 1, 0}));
	const Vec3 up = smf::cross(right, forward);

	std::fill(result, result + 16, 0.0F);

	result[15] = 1.0F;
	result[0] = right.x;
	result[1] = right.y;
	result[2] = right.z;
	result[3] = -smf::dot(right, eye);
	result[4] = up.x;
	result[5] = up.y;
	result[6] = up.z;
	result[7] = -smf::dot(up, eye);
	result[8] = -forward.x;
	result[9] = -forward.y;
	result[10] = -forward.z;
	result[11] = smf::dot(forward, eye);
}

inline void projectionMatrix(Matrix &result) {
	const float aspect = static_cast<float>(width) / height;
	const float distance = smf::length(camera.position());
	const float nearPlane = std::max(0.01F, distance - sceneRadius * 1.1F);
	const float farPlane = distance + sceneRadius * 2.0F;
	const float tangent = std::tan(halfFov) / std::min(aspect, 1.0F);

	std::fill(result, result + 16, 0.0F);

	result[0] = 1 / (aspect * tangent);
	result[5] = 1 / tangent;
	result[10] = -(farPlane + nearPlane) / (farPlane - nearPlane);
	result[11] = -2 * farPlane * nearPlane / (farPlane - nearPlane);
	result[14] = -1;
}

inline void modelMatrix(Matrix &result, Vec3 offset, float scale) {
	std::fill(result, result + 16, 0.0F);

	result[0] = result[5] = result[10] = scale;
	result[3] = offset.x;
	result[7] = offset.y;
	result[11] = offset.z;
	result[15] = 1;
}

inline void upload(Geometry &geometry, const std::vector<Vertex> &vertices,
                   bool dynamic = false) {
	if (!geometry.vao) {
		glGenVertexArrays(1, &geometry.vao);
		glGenBuffers(1, &geometry.vbo);
	}

	geometry.count = static_cast<GLsizei>(vertices.size());
	glBindVertexArray(geometry.vao);
	glBindBuffer(GL_ARRAY_BUFFER, geometry.vbo);
	glBufferData(GL_ARRAY_BUFFER,
	             static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
	             vertices.data(), dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

	const std::size_t offsets[] = {offsetof(Vertex, position),
	                               offsetof(Vertex, normal), offsetof(Vertex, uv)};
	const GLint components[] = {3, 3, 2};

	for (GLuint i = 0; i < 3; ++i) {
		glVertexAttribPointer(i, components[i], GL_FLOAT, GL_FALSE, sizeof(Vertex),
		                      reinterpret_cast<const void *>(offsets[i]));
		glEnableVertexAttribArray(i);
	}
}

inline void draw(const Geometry &geometry, int renderMode,
                 const std::array<float, 3> &color, Vec3 offset = {0, 0, 0},
                 float scale = 1) {
	Matrix model;
	modelMatrix(model, offset, scale);

	glUniformMatrix4fv(uniforms.model, 1, GL_TRUE, model);
	glUniform1i(uniforms.renderMode, renderMode);
	glUniform3fv(uniforms.diffuseColor, 1, color.data());

	glBindVertexArray(geometry.vao);

	glDrawArrays(GL_TRIANGLES, 0, geometry.count);
}

inline void setView() {
	Matrix view, projection;

	lookAt(view, target + camera.position(), target);
	projectionMatrix(projection);

	glUseProgram(program);

	glUniformMatrix4fv(uniforms.view, 1, GL_TRUE, view);
	glUniformMatrix4fv(uniforms.projection, 1, GL_TRUE, projection);
}

inline void clear() {
	glViewport(0, 0, width, height);

	glClearColor(0.035F, 0.045F, 0.065F, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	setView();
}

inline void reshape(int w, int h) {
	width = std::max(w, 1);
	height = std::max(h, 1);

	glViewport(0, 0, width, height);
	glutPostRedisplay();
}

inline void showHelp() {
	std::cout << "[Left/Right]: camera angle\n"
	             "[Up/Down]: camera height\n"
	             "[PgUp/PgDn]: camera radius decrease/increase\n"
	             "[R]: reset view\n"
	             "[H]: show help\n"
	             "[Esc]: close"
	          << std::endl;
}

inline void changed() {
	camera.angle = std::fmod(camera.angle, static_cast<float>(2 * M_PI));
	camera.radius =
	    std::clamp(camera.radius, sceneRadius * 1.2F, sceneRadius * 20.0F);
	camera.height =
	    std::clamp(camera.height, -sceneRadius * 20.0F, sceneRadius * 20.0F);

	glutPostRedisplay();
}

inline bool handleKey(unsigned char key) {
	switch (key) {
	case 27:
		glutLeaveMainLoop();
		return true;

	case 'r':
	case 'R':
		reset();
		changed();
		return true;

	default:
		return false;
	}
}

inline void special(int key, int, int) {
	switch (key) {
	case GLUT_KEY_LEFT:
	case GLUT_KEY_RIGHT:
		camera.angle += (key == GLUT_KEY_LEFT ? -1 : 1) * M_PI / 36;
		break;

	case GLUT_KEY_UP:
	case GLUT_KEY_DOWN:
		camera.height += (key == GLUT_KEY_UP ? 0.15F : -0.15F) * sceneRadius;
		break;

	case GLUT_KEY_PAGE_UP:
	case GLUT_KEY_PAGE_DOWN:
		camera.radius += (key == GLUT_KEY_PAGE_UP ? -0.15F : 0.15F) * sceneRadius;
		break;

	default:
		return;
	}

	changed();
}

inline GLuint loadShader(GLenum type, const std::filesystem::path &path) {
	std::ifstream file(path);
	const std::string source((std::istreambuf_iterator<char>(file)), {});

	const GLuint shader = glCreateShader(type);
	const char *sourcePointer = source.c_str();

	glShaderSource(shader, 1, &sourcePointer, nullptr);
	glCompileShader(shader);

	return shader;
}

inline void initialize(const std::filesystem::path &shaderDirectory) {
	glewExperimental = GL_TRUE;
	glewInit();
	glGetError();

	const GLuint vertex =
	    loadShader(GL_VERTEX_SHADER, shaderDirectory / "viewer.vert");
	const GLuint fragment =
	    loadShader(GL_FRAGMENT_SHADER, shaderDirectory / "viewer.frag");

	program = glCreateProgram();
	glAttachShader(program, vertex);
	glAttachShader(program, fragment);

	glLinkProgram(program);

	glDeleteShader(vertex);
	glDeleteShader(fragment);

	uniforms = {glGetUniformLocation(program, "model"),
	            glGetUniformLocation(program, "view"),
	            glGetUniformLocation(program, "projection"),
	            glGetUniformLocation(program, "renderMode"),
	            glGetUniformLocation(program, "diffuseColor")};

	glUseProgram(program);

	glUniform1i(glGetUniformLocation(program, "imageTexture"), 0);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_DITHER);
	glDisable(GL_MULTISAMPLE);
}

inline void destroyGeometry(Geometry &geometry) {
	glDeleteBuffers(1, &geometry.vbo);
	glDeleteVertexArrays(1, &geometry.vao);

	geometry = {};
}

inline void cleanup() {
	glDeleteProgram(program);

	program = 0;
}

inline void loadModel(Geometry &geometry, const std::string &path) {
	const smf::Mesh mesh = smf::loadSmf(path);

	std::vector<Vertex> vertices;
	vertices.reserve(mesh.triangles.size() * 3);

	for (const auto &face : mesh.triangles) {
		for (const auto index : face.indices) {
			vertices.push_back({(mesh.positions[index] - mesh.centroid) / mesh.radius,
			                    mesh.normals[index],
			                    {0, 0}});
		}
	}

	upload(geometry, vertices);
}

inline int createWindow(int argc, char **argv, const char *title) {
	const auto shaderDirectory =
	    std::filesystem::absolute(argv[0]).parent_path() / "shaders";

	int glutArgc = 1;

	glutInit(&glutArgc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	glutInitWindowSize(width, height);

	const int window = glutCreateWindow(title);
	initialize(shaderDirectory);

	return window;
}
} // namespace application
