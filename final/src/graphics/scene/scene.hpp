#pragma once
#include "graphics/resources/mesh.hpp"
#include "graphics/resources/shader.hpp"
#include "graphics/resources/shadow_map.hpp"
#include "game/render/draw_context.hpp"

namespace tanks {
class World;
}
namespace tanks::graphics {
struct Camera;
class SceneRenderer final : private DrawContext {
	public:
	explicit SceneRenderer(const std::string &shaderDirectory);
	void draw(const World &world, const M4 &viewProjection, const Camera &camera,
	          bool orthographic);

	private:
	struct Uniforms {
		GLint model = -1, viewProjection = -1, lightViewProjection = -1;
		GLint depthModel = -1, depthLightViewProjection = -1;
		GLint color = -1, light = -1, viewer = -1, shadow = -1, shadows = -1,
		      ambient = -1, specularStrength = -1, shininess = -1, lightEnabled = -1;
	} uniforms_;
	ShaderProgram scene_, depth_;
	Mesh cube_, cylinder_, arenaPrism_;
	ShadowMap shadow_;

	void geometry(const World &world);

	void cube(const M4 &model, Color color, Material material) override;

	void cylinder(const M4 &model, Color color, Material material) override;

	void arenaPrism(const M4 &model, Color color, Material material) override;

	void drawMesh(const Mesh &mesh, const M4 &model, Color color,
	              Material material);
};
} // namespace tanks::graphics
