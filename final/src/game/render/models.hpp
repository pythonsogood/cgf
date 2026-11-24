#pragma once
#include "game/render/mesh_renderer.hpp"

namespace tanks {
class ArenaRenderer final : public MeshRenderer {
	public:
	void draw(const World &world, DrawContext &draw) const override;
};

class CoverRenderer final : public MeshRenderer {
	public:
	void draw(const World &world, DrawContext &draw) const override;
};

class TankRenderer final : public MeshRenderer {
	public:
	void draw(const World &world, DrawContext &draw) const override;
};

class ShellRenderer final : public MeshRenderer {
	public:
	void draw(const World &world, DrawContext &draw) const override;
};

class BlastRenderer final : public MeshRenderer {
	public:
	void draw(const World &world, DrawContext &draw) const override;
};
} // namespace tanks
