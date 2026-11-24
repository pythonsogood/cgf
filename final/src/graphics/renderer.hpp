#pragma once
#include "core/math.hpp"
#include "graphics/frame_info.hpp"
#include "graphics/ui/hud_actions.hpp"
#include <memory>
#include <string>

namespace tanks {
class World;

class Renderer {
	public:
	explicit Renderer(const std::string &shaderDirectory);
	~Renderer();

	Renderer(const Renderer &) = delete;
	Renderer &operator=(const Renderer &) = delete;

	void draw(const World &world, const FrameInfo &frame);
	void resize(int width, int height);
	void orbit(float deltaX, float deltaY);
	void zoom(float steps);
	void resetCamera();
	Vec2 screenRight() const;

	graphics::HudActions takeHudActions();

	private:
	struct Impl;
	std::unique_ptr<Impl> impl_;
};
} // namespace tanks
