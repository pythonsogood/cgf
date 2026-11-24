#pragma once
#include "graphics/frame_info.hpp"
#include "graphics/ui/hud_actions.hpp"

struct ImGuiContext;

namespace tanks {
class World;
}

namespace tanks::graphics {
class HudRenderer {
	public:
	HudRenderer();
	~HudRenderer();

	HudRenderer(const HudRenderer &) = delete;
	HudRenderer &operator=(const HudRenderer &) = delete;

	void draw(const World &world, const FrameInfo &frame);

	HudActions takeActions();

	private:
	ImGuiContext *context_ = nullptr;
	bool rendererReady_ = false;
	HudActions actions_;
};
} // namespace tanks::graphics
