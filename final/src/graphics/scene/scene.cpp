#include "graphics/scene/scene.hpp"
#include "graphics/camera/camera.hpp"
#include "game/world/world.hpp"
#include <cmath>

namespace tanks::graphics {
SceneRenderer::SceneRenderer(const std::string &dir)
    : scene_(ShaderProgram::fromFiles(dir + "/scene.vert", dir + "/scene.frag",
				                                  "scene")),
      depth_(ShaderProgram::fromFiles(dir + "/depth.vert", dir + "/depth.frag",
						                                "depth")),
      cube_(Mesh::cube()), cylinder_(Mesh::cylinder()),
      arenaPrism_(Mesh::arenaPrism()) {
	uniforms_.model = scene_.uniform("uModel");
	uniforms_.viewProjection = scene_.uniform("uViewProjection");
	uniforms_.lightViewProjection = scene_.uniform("uLightViewProjection");
	uniforms_.color = scene_.uniform("uColor");
	uniforms_.light = scene_.uniform("uLightDirection");
	uniforms_.lightEnabled = scene_.uniform("uLightEnabled");
	uniforms_.viewer = scene_.uniform("uViewer");
	uniforms_.shadow = scene_.uniform("uShadowMap");
	uniforms_.shadows = scene_.uniform("uShadows");
	uniforms_.ambient = scene_.uniform("uAmbient");
	uniforms_.specularStrength = scene_.uniform("uSpecularStrength");
	uniforms_.shininess = scene_.uniform("uShininess");
	uniforms_.depthModel = depth_.uniform("uModel");
	uniforms_.depthLightViewProjection = depth_.uniform("uLightViewProjection");
}

namespace {
void drawObject(const GameObject &object, const World &world,
                DrawContext &context) {
	if (!object.active())
		return;

	for (auto *part = object.firstComponent(); part;
	     part = part->nextComponent()) {
		const auto *renderer = dynamic_cast<const MeshRenderer *>(part);
		if (renderer && renderer->visible)
			renderer->draw(world, context);
	}
}
} // namespace

void SceneRenderer::geometry(const World &world) {
	DrawContext &context = *this;
	world.forEachObject(drawObject, world, context);
}

void SceneRenderer::cube(const M4 &model, Color color, Material material) {
	drawMesh(cube_, model, color, material);
}

void SceneRenderer::cylinder(const M4 &model, Color color, Material material) {
	drawMesh(cylinder_, model, color, material);
}

void SceneRenderer::arenaPrism(const M4 &model, Color color,
                               Material material) {
	drawMesh(arenaPrism_, model, color, material);
}

void SceneRenderer::drawMesh(const Mesh &m, const M4 &model, Color c,
                             Material material) {
	if (depthPass_) {
		glUniformMatrix4fv(uniforms_.depthModel, 1, GL_FALSE, model.a);
		m.draw();
		return;
	}

	glUniformMatrix4fv(uniforms_.model, 1, GL_FALSE, model.a);
	glUniform3f(uniforms_.color, c.r, c.g, c.b);
	glUniform1f(uniforms_.ambient, material.ambient);
	glUniform1f(uniforms_.specularStrength, material.specularStrength);
	glUniform1f(uniforms_.shininess, material.shininess);
	m.draw();
}

void SceneRenderer::draw(const World &w, const M4 &vp, const Camera &camera,
                         bool orthographic) {
	const DirectionalLight &sun = w.light();
	const float horizontal = std::cos(sun.elevation);
	const V3 light{std::cos(sun.azimuth) * horizontal, std::sin(sun.elevation),
	               std::sin(sun.azimuth) * horizontal};
	const bool shadows = sun.active() && sun.castsShadows;
	const M4 lvp = multiply(ortho(-32, 32, -32, 32, .1f, 100),
	                        lookAt(mul(light, 55), {0, 0, 0}));

	if (shadows) {
		depthPass_ = true;
		glUseProgram(depth_.id());
		glUniformMatrix4fv(uniforms_.depthLightViewProjection, 1, GL_FALSE, lvp.a);
		glViewport(0, 0, 2048, 2048);
		glBindFramebuffer(GL_FRAMEBUFFER, shadow_.framebuffer());
		glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(2, 4);
		geometry(w);
		glDisable(GL_POLYGON_OFFSET_FILL);
	}

	depthPass_ = false;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, camera.width, camera.height);
	glUseProgram(scene_.id());
	glUniformMatrix4fv(uniforms_.viewProjection, 1, GL_FALSE, vp.a);
	glUniformMatrix4fv(uniforms_.lightViewProjection, 1, GL_FALSE, lvp.a);
	glUniform1i(uniforms_.lightEnabled, sun.active());
	glUniform3f(uniforms_.light, light.x, light.y, light.z);
	glUniform4f(uniforms_.viewer, camera.eye.x, camera.eye.y, camera.eye.z,
	            orthographic ? 0.0f : 1.0f);
	glUniform1i(uniforms_.shadows, shadows);
	glUniform1i(uniforms_.shadow, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shadow_.texture());
	geometry(w);
}
} // namespace tanks::graphics
