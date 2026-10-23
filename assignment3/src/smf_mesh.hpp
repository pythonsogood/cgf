#pragma once

#include <array>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace smf {

struct Vec3 {
	float x, y, z;
};

inline Vec3 operator+(Vec3 a, Vec3 b) {
	return {a.x + b.x, a.y + b.y, a.z + b.z};
}
inline Vec3 operator-(Vec3 a, Vec3 b) {
	return {a.x - b.x, a.y - b.y, a.z - b.z};
}
inline Vec3 operator/(Vec3 a, float scalar) {
	return {a.x / scalar, a.y / scalar, a.z / scalar};
}

inline float dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline Vec3 cross(Vec3 a, Vec3 b) {
	return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}
inline float length(Vec3 v) { return std::sqrt(dot(v, v)); }
inline Vec3 normalized(Vec3 v) { return v / length(v); }

struct Triangle {
	std::array<std::size_t, 3> indices;
	Vec3 normal;
};

struct Mesh {
	std::vector<Vec3> positions, normals;
	std::vector<Triangle> triangles;
	Vec3 centroid;
	float radius;
};

struct PendingFace {
	std::array<long long, 3> indices;
	std::size_t line;
};

inline Mesh loadSmf(const std::string &path) {
	std::ifstream input(path);

	if (!input) {
		throw std::runtime_error("Unable to open SMF");
	}

	Mesh mesh{};
	std::vector<PendingFace> faces;
	std::string line;

	for (std::size_t number = 1; std::getline(input, line); ++number) {
		std::istringstream record(line.substr(0, line.find('#')));
		std::string record_type;

		if (!(record >> record_type)) {
			continue;
		}

		if (record_type == "v") {
			Vec3 v{};
			std::string extra;

			if (!(record >> v.x >> v.y >> v.z) || (record >> extra)) {
				throw std::runtime_error(std::to_string(number) +
				                         ": vertex must contain exactly three coordinates");
			}

			mesh.positions.push_back(v);
		} else if (record_type == "f") {
			PendingFace face{};
			face.line = number;
			std::string extra;

			for (auto &index : face.indices) {
				if (!(record >> index)) {
					throw std::runtime_error(
					    std::to_string(number) +
					    ": face must contain exactly three vertex indices");
				}
			}

			if (record >> extra) {
				throw std::runtime_error(
				    std::to_string(number) +
				    ": face must contain exactly three vertex indices");
			}

			faces.push_back(face);
		} else {
			std::cout << std::to_string(number) + ": unrecognized record '" +
			                 record_type + "'"
			          << std::endl;
		}
	}

	if (input.bad()) {
		throw std::runtime_error("Error reading SMF");
	}

	if (mesh.positions.empty()) {
		throw std::runtime_error("no vertices");
	}

	if (faces.empty()) {
		throw std::runtime_error("no triangles");
	}

	mesh.triangles.reserve(faces.size());

	for (const auto &face : faces) {
		Triangle triangle{};

		for (std::size_t i = 0; i < 3; ++i) {
			const auto index = face.indices[i];

			if (index <= 0 ||
			    static_cast<unsigned long long>(index) > mesh.positions.size()) {
				throw std::runtime_error(std::to_string(face.line) +
				                         ": vertex index out of range");
			}

			triangle.indices[i] = static_cast<std::size_t>(index - 1);
		}

		const Vec3 a = mesh.positions[triangle.indices[0]];
		const Vec3 areaNormal = cross(mesh.positions[triangle.indices[1]] - a,
		                              mesh.positions[triangle.indices[2]] - a);
		const float area = length(areaNormal);

		if (!std::isfinite(area) || area == 0) {
			throw std::runtime_error(std::to_string(face.line) + ": invalid triangle");
		}

		triangle.normal = areaNormal / area;
		mesh.triangles.push_back(triangle);
	}

	mesh.normals.assign(mesh.positions.size(), Vec3{0, 0, 0});

	for (const auto &face : mesh.triangles) {
		for (const auto index : face.indices) {
			mesh.normals[index] = mesh.normals[index] + face.normal;
		}
	}

	for (std::size_t i = 0; i < mesh.normals.size(); ++i) {
		const float magnitude = length(mesh.normals[i]);

		if (!std::isfinite(magnitude) || magnitude == 0) {
			throw std::runtime_error("vertex " + std::to_string(i + 1) +
			                         " has invalid magnitude");
		}

		mesh.normals[i] = mesh.normals[i] / magnitude;
	}

	Vec3 sum{0, 0, 0};
	for (const auto position : mesh.positions) {
		sum = sum + position;
	}

	mesh.centroid = sum / static_cast<float>(mesh.positions.size());

	for (const auto position : mesh.positions) {
		const float distance = length(position - mesh.centroid);

		if (!std::isfinite(distance)) {
			throw std::runtime_error("mesh bounds are not finite");
		}

		if (distance > mesh.radius) {
			mesh.radius = distance;
		}
	}

	return mesh;
}

} // namespace smf
