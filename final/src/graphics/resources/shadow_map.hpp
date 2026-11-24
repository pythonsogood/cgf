#pragma once
#include <GL/glew.h>

namespace tanks::graphics {
class ShadowMap {
	public:
	ShadowMap();
	~ShadowMap();
	ShadowMap(const ShadowMap &) = delete;
	ShadowMap &operator=(const ShadowMap &) = delete;

	GLuint texture() const { return texture_; }
	GLuint framebuffer() const { return framebuffer_; }

	private:
	GLuint framebuffer_ = 0, texture_ = 0;
};
} // namespace tanks::graphics
