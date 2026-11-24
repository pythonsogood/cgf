#include "graphics/resources/mesh.hpp"
#include "graphics/camera/math.hpp"
#include "game/world/arena.hpp"
#include <cmath>
#include <vector>
namespace tanks::graphics {
namespace {
void triangle(std::vector<float> &v, V3 a, V3 b, V3 c, V3 n) {
	for (V3 p : {a, b, c})
		v.insert(v.end(), {p.x, p.y, p.z, n.x, n.y, n.z});
}

Mesh makeMesh(const std::vector<float> &d) {
	GLuint vao = 0, vbo = 0;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER,
	             static_cast<GLsizeiptr>(d.size() * sizeof(float)), d.data(),
	             GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
	                      reinterpret_cast<void *>(3 * sizeof(float)));
	glBindVertexArray(0);

	return Mesh(vao, vbo, static_cast<GLsizei>(d.size() / 6));
}
} // namespace
Mesh::~Mesh() {
	if (vbo_)
		glDeleteBuffers(1, &vbo_);

	if (vao_)
		glDeleteVertexArrays(1, &vao_);
}

void Mesh::draw() const {
	glBindVertexArray(vao_);
	glDrawArrays(GL_TRIANGLES, 0, count_);
}

Mesh Mesh::cube() {
	std::vector<float> v;
	const V3 p[8] = {{-.5f, -.5f, -.5f}, {.5f, -.5f, -.5f}, {.5f, .5f, -.5f},
	                 {-.5f, .5f, -.5f},  {-.5f, -.5f, .5f}, {.5f, -.5f, .5f},
	                 {.5f, .5f, .5f},    {-.5f, .5f, .5f}};

	const int f[6][4] = {{0, 3, 2, 1}, {4, 5, 6, 7}, {0, 4, 7, 3},
	                     {1, 2, 6, 5}, {3, 7, 6, 2}, {0, 1, 5, 4}};

	for (int i = 0; i < 6; ++i) {
		V3 n = norm(cross(sub(p[f[i][1]], p[f[i][0]]), sub(p[f[i][2]], p[f[i][0]])));
		triangle(v, p[f[i][0]], p[f[i][1]], p[f[i][2]], n);
		triangle(v, p[f[i][0]], p[f[i][2]], p[f[i][3]], n);
	}

	return makeMesh(v);
}

Mesh Mesh::cylinder() {
	std::vector<float> v;
	constexpr int sides = 16;
	v.reserve(sides * 12 * 6);

	for (int i = 0; i < sides; ++i) {
		float a = 2 * pi * i / sides, b = 2 * pi * (i + 1) / sides;
		V3 p0{std::cos(a) * .5f, -.5f, std::sin(a) * .5f},
		    p1{std::cos(b) * .5f, -.5f, std::sin(b) * .5f}, p2{p1.x, .5f, p1.z},
		    p3{p0.x, .5f, p0.z};

		for (V3 p : {p0, p2, p1, p0, p3, p2})
			v.insert(v.end(), {p.x, p.y, p.z, 2 * p.x, 0.0f, 2 * p.z});

		triangle(v, {0, -.5f, 0}, p0, p1, {0, -1, 0});
		triangle(v, {0, .5f, 0}, p2, p3, {0, 1, 0});
	}

	return makeMesh(v);
}

Mesh Mesh::arenaPrism() {
	std::vector<float> v;
	const auto &outline = tanks::unitArenaOutline();
	constexpr float bottom = -.5f, top = .5f;
	v.reserve(tanks::arenaSides * 12 * 6);

	for (std::size_t i = 0; i < tanks::arenaSides; ++i) {
		const auto &a = outline[i];
		const auto &b = outline[(i + 1) % tanks::arenaSides];
		const V3 lowerA{a.x, bottom, a.z}, lowerB{b.x, bottom, b.z};
		const V3 upperA{a.x, top, a.z}, upperB{b.x, top, b.z};
		const V3 sideNormal = norm({b.z - a.z, 0, a.x - b.x});

		triangle(v, lowerA, lowerB, upperB, sideNormal);
		triangle(v, lowerA, upperB, upperA, sideNormal);
		triangle(v, {0, top, 0}, upperB, upperA, {0, 1, 0});
		triangle(v, {0, bottom, 0}, lowerA, lowerB, {0, -1, 0});
	}

	return makeMesh(v);
}
} // namespace tanks::graphics
