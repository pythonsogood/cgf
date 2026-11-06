#pragma once

#include <algorithm>
#include <cmath>
#include <vector>

namespace texture {

inline std::vector<unsigned char> generate(int size) {
	if (size <= 0) {
		return {};
	}

	std::vector<unsigned char> pixels(static_cast<std::size_t>(size) * size * 3);
	const float scale = 6.0f;

	for (int y = 0; y < size; ++y) {
		for (int x = 0; x < size; ++x) {
			float u = (static_cast<float>(x) + 0.5f) / size;
			float v = (static_cast<float>(y) + 0.5f) / size;

			float gx = u * scale;
			float gy = v * scale;

			int cellX = static_cast<int>(std::floor(gx));
			int cellY = static_cast<int>(std::floor(gy));

			float px = gx - cellX - 0.5f;
			float py = gy - cellY - 0.5f;

			float angle = std::atan2(py, px);
			float radius = std::sqrt(px * px + py * py);

			float phase = 0.31f * std::sin(cellX * 1.73f + cellY * 0.91f);
			float petals = std::cos(8.0f * angle + phase);
			float outline = radius - (0.29f + 0.055f * petals);
			float ring = std::cos(2.0f * static_cast<float>(M_PI) *
			                      (radius * 4.1f + 0.08f * petals));
			float grain =
			    0.5f + 0.5f * std::sin(19.0f * gx + 7.0f * std::sin(11.0f * gy));
			float motif = 1.0f - std::clamp(std::abs(outline) * 9.0f, 0.0f, 1.0f);
			float center = 1.0f - std::clamp(radius * 3.6f, 0.0f, 1.0f);
			float variation = 0.5f + 0.5f * ring;

			float red = 0.20f + 0.48f * motif + 0.18f * center + 0.10f * variation;
			float green = 0.16f + 0.28f * motif + 0.35f * variation + 0.12f * grain;
			float blue =
			    0.30f + 0.33f * motif + 0.25f * (1.0f - variation) + 0.10f * center;

			std::size_t index = (static_cast<std::size_t>(y) * size + x) * 3;

			pixels[index] =
			    static_cast<unsigned char>(std::clamp(red, 0.0f, 1.0f) * 255.0f);
			pixels[index + 1] =
			    static_cast<unsigned char>(std::clamp(green, 0.0f, 1.0f) * 255.0f);
			pixels[index + 2] =
			    static_cast<unsigned char>(std::clamp(blue, 0.0f, 1.0f) * 255.0f);
		}
	}

	return pixels;
}

} // namespace texture
