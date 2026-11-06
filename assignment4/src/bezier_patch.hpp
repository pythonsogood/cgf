#pragma once

#include "smf_mesh.hpp"

#include <array>
#include <cmath>
#include <vector>

namespace bezier {

using ControlPoints = std::array<smf::Vec3, 16>;

struct Vertex {
	smf::Vec3 position;
	smf::Vec3 normal;
	std::array<float, 2> uv;
};

namespace detail {

inline smf::Vec3 safeNormal(smf::Vec3 value) {
	const float magnitudeSquared = smf::dot(value, value);
	if (magnitudeSquared <= 0.0F || !std::isfinite(magnitudeSquared)) {
		return {0, 0, 1};
	}

	return value / std::sqrt(magnitudeSquared);
}

inline void basis(float t, float (&weights)[4], float (&derivatives)[4]) {
	const float s = 1.0f - t; // (1 - t)
	const float s2 = s * s;   // (1 - t)^2
	const float t2 = t * t;   // t^2

	weights[0] = s2 * s;        // (1 - t)^3
	weights[1] = 3.0f * t * s2; // 3t * (1 - t)^2
	weights[2] = 3.0f * t2 * s; // 3t^2 * (1 - t)
	weights[3] = t2 * t;        // t^3

	derivatives[0] = -3.0f * s2;               // -3 * (1 - t)^2
	derivatives[1] = 3.0f * s2 - 6.0f * t * s; // 3 * (1 - t)^2 - 6t * (1 - t)
	derivatives[2] = 6.0f * t * s - 3.0f * t2; // 6t * (1 - t) - 3t^2
	derivatives[3] = 3.0f * t2;                // 3t^2
}

inline void emitTriangle(std::vector<Vertex> &triangles, bool flat,
                         const Vertex &a, const Vertex &b, const Vertex &c) {
	if (flat) {
		const smf::Vec3 faceNormal =
		    safeNormal(smf::cross(b.position - a.position, c.position - a.position));

		triangles.push_back({a.position, faceNormal, a.uv});
		triangles.push_back({b.position, faceNormal, b.uv});
		triangles.push_back({c.position, faceNormal, c.uv});
	} else {
		triangles.push_back(a);
		triangles.push_back(b);
		triangles.push_back(c);
	}
}

} // namespace detail

inline ControlPoints initialPoints() {
	return {{{0, 0, 0},
	         {2, 0, 1.5F},
	         {4, 0, 2.9F},
	         {6, 0, 0},
	         {0, 2, 1.1F},
	         {2, 2, 3.9F},
	         {4, 2, 3.1F},
	         {6, 2, 0.7F},
	         {0, 4, -0.5F},
	         {2, 4, 2.6F},
	         {4, 4, 2.4F},
	         {6, 4, 0.4F},
	         {0, 6, 0.3F},
	         {2, 6, -1.1F},
	         {4, 6, 1.3F},
	         {6, 6, -0.2F}}};
}

inline Vertex evaluate(const ControlPoints &points, float u, float v) {
	float bu[4], dbu[4], bv[4], dbv[4];
	detail::basis(u, bu, dbu);
	detail::basis(v, bv, dbv);

	smf::Vec3 position{}, tangentU{}, tangentV{};

	for (int row = 0; row < 4; ++row) {
		for (int column = 0; column < 4; ++column) {
			const smf::Vec3 point = points[row * 4 + column];
			const float weight = bu[column] * bv[row];

			position = position +
			           smf::Vec3{point.x * weight, point.y * weight, point.z * weight};

			const float weightU = dbu[column] * bv[row];

			tangentU = tangentU + smf::Vec3{point.x * weightU, point.y * weightU,
			                                point.z * weightU};

			const float weightV = bu[column] * dbv[row];
			tangentV = tangentV + smf::Vec3{point.x * weightV, point.y * weightV,
			                                point.z * weightV};
		}
	}

	const smf::Vec3 normal = detail::safeNormal(smf::cross(tangentU, tangentV));

	return {position, normal, {u, v}};
}

inline std::vector<Vertex> tessellate(const ControlPoints &points,
                                      int resolution, bool flat) {
	if (resolution <= 0) {
		return {};
	}

	const std::size_t side = static_cast<std::size_t>(resolution) + 1;

	std::vector<Vertex> samples;
	samples.reserve(side * side);

	for (int row = 0; row <= resolution; ++row) {
		const float v = static_cast<float>(row) / resolution;

		for (int column = 0; column <= resolution; ++column) {
			const float u = static_cast<float>(column) / resolution;

			samples.push_back(evaluate(points, u, v));
		}
	}

	std::vector<Vertex> triangles;
	triangles.reserve(6 * (side - 1) * (side - 1));

	for (int row = 0; row < resolution; ++row) {
		for (int column = 0; column < resolution; ++column) {
			const std::size_t topLeft = row * side + column;

			const Vertex &p00 = samples[topLeft];
			const Vertex &p10 = samples[topLeft + 1];
			const Vertex &p01 = samples[topLeft + side];
			const Vertex &p11 = samples[topLeft + side + 1];

			detail::emitTriangle(triangles, flat, p00, p10, p01);
			detail::emitTriangle(triangles, flat, p10, p11, p01);
		}
	}

	return triangles;
}

} // namespace bezier
