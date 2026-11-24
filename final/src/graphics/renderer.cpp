#include <GL/glew.h>
#include "graphics/renderer.hpp"
#include "game/world/world.hpp"
#include "graphics/camera/camera.hpp"
#include "graphics/ui/hud.hpp"
#include "graphics/scene/scene.hpp"
#include <memory>

namespace tanks {
struct Renderer::Impl {
	graphics::Camera camera;
	graphics::SceneRenderer scene;
	graphics::HudRenderer hud;
	explicit Impl(const std::string &dir) : scene(dir) {}

	void draw(const World &w, const FrameInfo &f) {
		camera.resize(f.width, f.height);
		glViewport(0, 0, camera.width, camera.height);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);
		glClearColor(.075f, .105f, .13f, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		const auto vp = camera.viewProjection(f.orthographic);
		scene.draw(w, vp, camera, f.orthographic);
		hud.draw(w, f);
	}
};

Renderer::Renderer(const std::string &shaderDirectory)
    : impl_(new Impl(shaderDirectory)) {}
Renderer::~Renderer() = default;

void Renderer::draw(const World &w, const FrameInfo &f) { impl_->draw(w, f); }

void Renderer::resize(int w, int h) {
	impl_->camera.resize(w, h);
	glViewport(0, 0, impl_->camera.width, impl_->camera.height);
}

void Renderer::orbit(float dx, float dy) { impl_->camera.orbit(dx, dy); }

void Renderer::zoom(float steps) { impl_->camera.zoom(steps); }

void Renderer::resetCamera() { impl_->camera.reset(); }

Vec2 Renderer::screenRight() const { return impl_->camera.screenRight(); }

graphics::HudActions Renderer::takeHudActions() {
	return impl_->hud.takeActions();
}
} // namespace tanks
