#include "graphics/resources/shadow_map.hpp"
#include <stdexcept>
namespace tanks::graphics {
ShadowMap::ShadowMap() {
	glGenFramebuffers(1, &framebuffer_);
	glGenTextures(1, &texture_);
	glBindTexture(GL_TEXTURE_2D, texture_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, 2048, 2048, 0,
	             GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	const float border[] = {1, 1, 1, 1};
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
	                       texture_, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		if (texture_) {
			glDeleteTextures(1, &texture_);
			texture_ = 0;
		}

		if (framebuffer_) {
			glDeleteFramebuffers(1, &framebuffer_);
			framebuffer_ = 0;
		}

		throw std::runtime_error("Directional shadow framebuffer is incomplete");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

ShadowMap::~ShadowMap() {
	if (texture_)
		glDeleteTextures(1, &texture_);

	if (framebuffer_)
		glDeleteFramebuffers(1, &framebuffer_);
}
} // namespace tanks::graphics
