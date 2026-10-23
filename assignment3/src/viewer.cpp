#include <GL/glew.h>
#include <GL/freeglut.h>

#include "smf_mesh.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

namespace {
using Matrix = float[16];
using smf::Vec3;
constexpr float halfFov = M_PI / 8.0F;

struct Material {
	const char *name;
	std::array<float, 4> ambient, diffuse, specular;
	float shininess;
};

constexpr Material materials[] = {{"Red polished",
                                   {0.6F, 0.2F, 0.2F, 1},
                                   {0.9F, 0.1F, 0.1F, 1},
                                   {0.8F, 0.8F, 0.8F, 1},
                                   80},
                                  {"Blue matte",
                                   {0.04F, 0.10F, 0.20F, 1},
                                   {0.12F, 0.42F, 0.80F, 1},
                                   {0.04F, 0.04F, 0.04F, 1},
                                   8},
                                  {"Gold metal",
                                   {0.24725F, 0.1995F, 0.0745F, 1},
                                   {0.75164F, 0.60648F, 0.22648F, 1},
                                   {0.628281F, 0.555802F, 0.366065F, 1},
                                   51.2F}};

const char *shadingNames[] = {"Flat normals", "Gouraud", "Phong"};

struct Cylinder {
	float angle, radius, height;

	Vec3 position() const {
		return {radius * std::cos(angle), height, radius * std::sin(angle)};
	}
};

struct Vertex {
	Vec3 position, normal, faceNormal;
};

struct Uniforms {
	GLint view, projection, shading, ambient, diffuse, specular, shininess;
	GLint objectLight, cameraLight, perspectiveProjection;
} uniforms{};

GLuint program = 0, vao = 0, vbo = 0;
GLsizei vertexCount = 0;
Cylinder camera{}, light{};
int shading = 0, material = 0, width = 1000, height = 800;
bool perspective = true;

void reset() {
	camera = {static_cast<float>(M_PI / 2), 3.3F, 0.55F};
	light = {static_cast<float>(M_PI / 4), 3.0F, 2.0F};
	shading = 0;
	material = 0;
	perspective = true;
}

void lookAt(Matrix &result, Vec3 eye, Vec3 target) {
	const Vec3 forward = smf::normalized(target - eye);
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

Vec3 transformPoint(const Matrix &m, Vec3 v) {
	return {m[0] * v.x + m[1] * v.y + m[2] * v.z + m[3],
	        m[4] * v.x + m[5] * v.y + m[6] * v.z + m[7],
	        m[8] * v.x + m[9] * v.y + m[10] * v.z + m[11]};
}

void projectionMatrix(Matrix &result) {
	const float aspect = static_cast<float>(width) / height;
	const float distance = smf::length(camera.position());
	const float nearPlane = std::max(0.01F, distance - 1.1F);
	const float farPlane = distance + 3.0F;
	const float tangent = std::tan(halfFov) / std::min(aspect, 1.0F);

	std::fill(result, result + 16, 0.0F);

	if (perspective) {
		result[0] = 1 / (aspect * tangent);
		result[5] = 1 / tangent;
		result[10] = -(farPlane + nearPlane) / (farPlane - nearPlane);
		result[11] = -2 * farPlane * nearPlane / (farPlane - nearPlane);
		result[14] = -1;
	} else {
		const float extent = distance * tangent;

		result[0] = 1 / (extent * aspect);
		result[5] = 1 / extent;
		result[10] = -2 / (farPlane - nearPlane);
		result[11] = -(farPlane + nearPlane) / (farPlane - nearPlane);
		result[15] = 1;
	}
}

void showHelp() {
	std::cout << "[0 / 1 / 2]: flat normal colors / Gouraud / Phong\n"
	             "[P / O]: perspective / parallel (orthographic) projection\n"
	             "[3 / 4 / 5] : red / matte blue / gold\n"
	             "[Left/Right]: camera cylinder angle\n"
	             "[Up/Down]: camera height\n"
	             "[PgUp/PgDn]: camera radius decrease/increase\n"
	             "[J / L]: object light cylinder angle decrease/increase\n"
	             "[I / K]: object light height increase/decrease\n"
	             "[U / N]: object light radius decrease/increase\n"
	             "[R]: reset camera, light, shading, projection, material\n"
	             "[H]: show help\n"
	             "[Esc]: close"
	          << std::endl;
}

void showStatus() {
	std::cout << shadingNames[shading] << " | "
	          << (perspective ? "Perspective" : "Parallel") << " | "
	          << materials[material].name << std::fixed << std::setprecision(2)
	          << " | Camera Angle/Radius/Height " << camera.angle * 180 / M_PI
	          << '/' << camera.radius << '/' << camera.height
	          << " | Light Angle/Radius/Height " << light.angle * 180 / M_PI << '/'
	          << light.radius << '/' << light.height << std::endl;
}

void display() {
	Matrix view, projection;

	lookAt(view, camera.position(), {0, 0, 0});
	projectionMatrix(projection);

	const Vec3 objectLight = transformPoint(view, light.position());
	const Material &selected = materials[material];

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(program);

	glUniformMatrix4fv(uniforms.view, 1, GL_TRUE, view);
	glUniformMatrix4fv(uniforms.projection, 1, GL_TRUE, projection);
	glUniform1i(uniforms.shading, shading);
	glUniform1i(uniforms.perspectiveProjection, perspective);
	glUniform4fv(uniforms.ambient, 1, selected.ambient.data());
	glUniform4fv(uniforms.diffuse, 1, selected.diffuse.data());
	glUniform4fv(uniforms.specular, 1, selected.specular.data());
	glUniform1f(uniforms.shininess, selected.shininess);
	glUniform3f(uniforms.objectLight, objectLight.x, objectLight.y, objectLight.z);
	glUniform3f(uniforms.cameraLight, -0.4F, 0.6F, 0.4F);

	glBindVertexArray(vao);

	glDrawArrays(GL_TRIANGLES, 0, vertexCount);

	glutSwapBuffers();
}

void reshape(int w, int h) {
	width = std::max(w, 1);
	height = std::max(h, 1);

	glViewport(0, 0, width, height);
	glutPostRedisplay();
}

void constrain(Cylinder &c, float minimumRadius) {
	if (c.angle < 0 || c.angle >= 2 * M_PI) {
		c.angle = std::fmod(c.angle, 2 * M_PI);

		if (c.angle < 0) {
			c.angle += 2 * M_PI;
		}
	}

	c.radius = std::clamp(c.radius, minimumRadius, 20.0F);
	c.height = std::clamp(c.height, -20.0F, 20.0F);
}

void changed() {
	constrain(camera, 1.2F);
	constrain(light, 0.1F);

	showStatus();
	glutPostRedisplay();
}

void keyboard(unsigned char key, int, int) {
	switch (key) {
	// ESC
	case 27:
		glutLeaveMainLoop();
		return;

	case '0':
	case '1':
	case '2':
		shading = key - '0';
		break;

	case '3':
	case '4':
	case '5':
		material = key - '3';
		break;

	case 'p':
		perspective = true;
		break;

	case 'o':
		perspective = false;
		break;

	case 'j':
	case 'l':
		light.angle += (key == 'j' ? -1 : 1) * M_PI / 36;
		break;

	case 'i':
	case 'k':
		light.height += key == 'i' ? 0.15F : -0.15F;
		break;

	case 'u':
	case 'n':
		light.radius += key == 'u' ? -0.15F : 0.15F;
		break;

	case 'r':
		reset();
		break;

	case 'h':
		showHelp();
		return;

	default:
		return;
	}

	changed();
}

void special(int key, int, int) {
	switch (key) {
	case GLUT_KEY_LEFT:
	case GLUT_KEY_RIGHT:
		camera.angle += (key == GLUT_KEY_LEFT ? -1 : 1) * M_PI / 36;
		break;

	case GLUT_KEY_UP:
	case GLUT_KEY_DOWN:
		camera.height += key == GLUT_KEY_UP ? 0.15F : -0.15F;
		break;

	case GLUT_KEY_PAGE_UP:
	case GLUT_KEY_PAGE_DOWN:
		camera.radius += key == GLUT_KEY_PAGE_UP ? -0.15F : 0.15F;
		break;

	default:
		return;
	}

	changed();
}

GLuint loadShader(GLenum type, const std::filesystem::path &path) {
	std::ifstream file(path);
	const std::string source((std::istreambuf_iterator<char>(file)), {});

	const GLuint shader = glCreateShader(type);
	const char *sourcePointer = source.c_str();

	glShaderSource(shader, 1, &sourcePointer, nullptr);
	glCompileShader(shader);

	return shader;
}

void initialize(const smf::Mesh &mesh,
                const std::filesystem::path &shaderDirectory) {
	glewExperimental = GL_TRUE;
	glewInit();

	GLuint vertex = loadShader(GL_VERTEX_SHADER, shaderDirectory / "viewer.vert");
	GLuint fragment =
	    loadShader(GL_FRAGMENT_SHADER, shaderDirectory / "viewer.frag");

	program = glCreateProgram();
	glAttachShader(program, vertex);
	glAttachShader(program, fragment);

	glLinkProgram(program);

	glDeleteShader(vertex);
	glDeleteShader(fragment);

	uniforms = {glGetUniformLocation(program, "view"),
	            glGetUniformLocation(program, "projection"),
	            glGetUniformLocation(program, "shadingMode"),
	            glGetUniformLocation(program, "materialAmbient"),
	            glGetUniformLocation(program, "materialDiffuse"),
	            glGetUniformLocation(program, "materialSpecular"),
	            glGetUniformLocation(program, "materialShininess"),
	            glGetUniformLocation(program, "objectLightEye"),
	            glGetUniformLocation(program, "cameraLightEye"),
	            glGetUniformLocation(program, "perspectiveProjection")};

	glUseProgram(program);

	glUniform4f(glGetUniformLocation(program, "lightAmbient"), 0.2F, 0.2F, 0.2F,
	            1);
	glUniform4f(glGetUniformLocation(program, "lightDiffuse"), 0.6F, 0.6F, 0.6F,
	            1);
	glUniform4f(glGetUniformLocation(program, "lightSpecular"), 1, 1, 1, 1);

	std::vector<Vertex> vertices;
	vertices.reserve(mesh.triangles.size() * 3);

	for (const auto &face : mesh.triangles) {
		for (const auto index : face.indices) {
			vertices.push_back({(mesh.positions[index] - mesh.centroid) / mesh.radius,
			                    mesh.normals[index], face.normal});
		}
	}

	vertexCount = static_cast<GLsizei>(vertices.size());

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER,
	             static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
	             vertices.data(), GL_STATIC_DRAW);

	const std::size_t offsets[] = {offsetof(Vertex, position),
	                               offsetof(Vertex, normal),
	                               offsetof(Vertex, faceNormal)};

	for (GLuint i = 0; i < 3; ++i) {
		glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
		                      reinterpret_cast<const void *>(offsets[i]));
		glEnableVertexAttribArray(i);
	}

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.035F, 0.045F, 0.065F, 1);
}

void cleanup() {
	if (vbo) {
		glDeleteBuffers(1, &vbo);
	}

	if (vao) {
		glDeleteVertexArrays(1, &vao);
	}

	if (program) {
		glDeleteProgram(program);
	}

	vbo = vao = program = 0;
}
} // namespace

int main(int argc, char **argv) {
	if (argc == 2 &&
	    (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")) {
		std::cout << "Usage: viewer <model.smf>" << std::endl;
		showHelp();

		return 0;
	}

	if (argc != 2) {
		std::cerr << "Usage: viewer <model.smf>" << std::endl;

		return 1;
	}

	int window = 0;

	const smf::Mesh mesh = smf::loadSmf(argv[1]);
	std::string modelName = std::filesystem::path(argv[1]).filename().string();

	std::cout << mesh.positions.size() << " vertices" << std::endl;

	const auto shaderDirectory =
	    std::filesystem::absolute(argv[0]).parent_path() / "shaders";

	int glutArgc = 1;

	glutInit(&glutArgc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutInitWindowSize(width, height);

	window = glutCreateWindow(modelName.c_str());

	initialize(mesh, shaderDirectory);
	reset();
	showHelp();
	showStatus();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
	glutCloseFunc(cleanup);

	glutMainLoop();

	return 0;
}
