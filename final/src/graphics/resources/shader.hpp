#pragma once
#include <GL/glew.h>
#include <string>

namespace tanks::graphics {
class ShaderProgram {
	public:
	explicit ShaderProgram(GLuint id);
	~ShaderProgram();
	ShaderProgram(const ShaderProgram &) = delete;
	ShaderProgram &operator=(const ShaderProgram &) = delete;

	GLuint id() const { return id_; }

	GLint uniform(const char *name) const;

	static ShaderProgram fromFiles(const std::string &vertexPath,
	                               const std::string &fragmentPath,
	                               const std::string &label);

	private:
	GLuint id_ = 0;
};
} // namespace tanks::graphics
