#pragma once

#include "application.hpp"

#include <sstream>
#include <stdexcept>

namespace patch_scene {

struct Patch {
	bezier::ControlPoints points = bezier::initialPoints();
	application::Geometry surface{}, guides{};
	int resolution = 10, selectedPoint = 0;
};

enum class Edit { None, Selection, Geometry };

inline void update(Patch &patch, bool flat) {
	using application::Vec3;

	application::target = {};

	for (const auto point : patch.points) {
		application::target = application::target + point;
	}

	application::target = application::target / 16.0F;
	application::sceneRadius = 0.1F;

	for (const auto point : patch.points) {
		application::sceneRadius = std::max(application::sceneRadius,
		                                    smf::length(point - application::target));
	}

	application::upload(patch.surface,
	                    bezier::tessellate(patch.points, patch.resolution, flat),
	                    true);

	std::vector<bezier::Vertex> vertices;
	vertices.reserve(22);

	for (const auto point : patch.points) {
		vertices.push_back({point, {0, 0, 1}, {0, 0}});
	}

	for (const Vec3 end : {Vec3{0.4F, 0, 0}, Vec3{0, 0.4F, 0}, Vec3{0, 0, 0.4F}}) {
		vertices.push_back({{0, 0, 0}, {0, 0, 1}, {0, 0}});
		vertices.push_back({end, {0, 0, 1}, {0, 0}});
	}

	application::upload(patch.guides, vertices, true);
}

inline void drawGuides(const Patch &patch) {
	application::Matrix model;
	application::modelMatrix(model, {0, 0, 0}, 1);

	glUniformMatrix4fv(application::uniforms.model, 1, GL_TRUE, model);
	glUniform1i(application::uniforms.renderMode, application::unlit);
	glBindVertexArray(patch.guides.vao);
	glDisable(GL_DEPTH_TEST);

	const float colors[][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
	for (int i = 0; i < 3; ++i) {
		glUniform3fv(application::uniforms.diffuseColor, 1, colors[i]);
		glDrawArrays(GL_LINES, 16 + i * 2, 2);
	}

	glPointSize(6);
	glUniform3f(application::uniforms.diffuseColor, 0.95F, 0.3F, 0.25F);

	glDrawArrays(GL_POINTS, 0, 16);

	glPointSize(12);
	glUniform3f(application::uniforms.diffuseColor, 1, 0.95F, 0.1F);

	glDrawArrays(GL_POINTS, patch.selectedPoint, 1);

	glEnable(GL_DEPTH_TEST);
}

inline Edit edit(Patch &patch, unsigned char key) {
	auto &point = patch.points[patch.selectedPoint];

	switch (key) {
	case '[':
	case ']':
		patch.selectedPoint = (patch.selectedPoint + (key == '[' ? 15 : 1)) % 16;
		return Edit::Selection;

	case 'a':
	case 'd':
		point.x += key == 'a' ? -0.1F : 0.1F;
		break;

	case 's':
	case 'w':
		point.y += key == 's' ? -0.1F : 0.1F;
		break;

	case 'q':
	case 'e':
		point.z += key == 'q' ? -0.1F : 0.1F;
		break;

	case '-':
	case '_':
	case '+':
	case '=':
		patch.resolution = std::clamp(
		    patch.resolution + (key == '-' || key == '_' ? -1 : 1), 1, 100);
		break;

	default:
		return Edit::None;
	}

	return Edit::Geometry;
}

inline void loadControlPoints(Patch &patch, const std::string &path) {
	std::ifstream input(path);

	if (!input) {
		throw std::runtime_error("Unable to open control points: " + path);
	}

	std::string line;
	for (auto &point : patch.points) {
		if (!std::getline(input, line)) {
			throw std::runtime_error("Expected 16 lines of control points");
		}

		std::istringstream record(line);
		std::string extra;

		if (!(record >> point.x >> point.y >> point.z) || record >> extra ||
		    !std::isfinite(point.x) || !std::isfinite(point.y) ||
		    !std::isfinite(point.z)) {
			throw std::runtime_error(
			    "Each control point needs three finite coordinates");
		}
	}

	while (std::getline(input, line)) {
		if (line.find_first_not_of(" \t\r") != std::string::npos) {
			throw std::runtime_error("Expected exactly 16 control points");
		}
	}
}

inline void showHelp() {
	std::cout << "[[ / ]]: previous/next control point\n"
	             "[A / D]: selected point X decrease/increase\n"
	             "[S / W]: selected point Y decrease/increase\n"
	             "[Q / E]: selected point Z decrease/increase\n"
	             "[- / +]: tessellation decrease/increase (1..100)\n";
}

inline void cleanup(Patch &patch) {
	application::destroyGeometry(patch.surface);
	application::destroyGeometry(patch.guides);
}
} // namespace patch_scene
