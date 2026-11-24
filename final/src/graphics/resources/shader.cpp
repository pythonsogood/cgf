#include "graphics/resources/shader.hpp"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>
namespace tanks::graphics {
namespace {
std::string loadFile(const std::string &p) {
	std::ifstream f(p, std::ios::binary);
	if (!f)
		throw std::runtime_error("Cannot open shader: " + p);
	std::ostringstream s;
	s << f.rdbuf();
	return s.str();
}

GLuint compile(GLenum type, const std::string &src, const std::string &name) {
	GLuint s = glCreateShader(type);
	const char *p = src.c_str();
	glShaderSource(s, 1, &p, nullptr);
	glCompileShader(s);
	GLint ok = 0;
	glGetShaderiv(s, GL_COMPILE_STATUS, &ok);

	if (!ok) {
		GLint n = 0;
		glGetShaderiv(s, GL_INFO_LOG_LENGTH, &n);
		std::string log(static_cast<size_t>(std::max(n, 1)), 0);
		glGetShaderInfoLog(s, n, nullptr, log.data());
		glDeleteShader(s);
		throw std::runtime_error("Shader compile failed [" + name + "]: " + log);
	}

	return s;
}
} // namespace
ShaderProgram::ShaderProgram(GLuint id) : id_(id) {}
ShaderProgram::~ShaderProgram() {
	if (id_)
		glDeleteProgram(id_);
}

GLint ShaderProgram::uniform(const char *n) const {
	return glGetUniformLocation(id_, n);
}

ShaderProgram ShaderProgram::fromFiles(const std::string &v,
                                       const std::string &f,
                                       const std::string &label) {
	GLuint vs = compile(GL_VERTEX_SHADER, loadFile(v), label + " vertex");
	GLuint fs = 0, p = 0;

	try {
		fs = compile(GL_FRAGMENT_SHADER, loadFile(f), label + " fragment");
		p = glCreateProgram();
		glAttachShader(p, vs);
		glAttachShader(p, fs);
		glLinkProgram(p);
		glDeleteShader(vs);
		vs = 0;
		glDeleteShader(fs);
		fs = 0;
		GLint ok = 0;
		glGetProgramiv(p, GL_LINK_STATUS, &ok);
		if (!ok) {
			GLint n = 0;
			glGetProgramiv(p, GL_INFO_LOG_LENGTH, &n);
			std::string log(static_cast<size_t>(std::max(n, 1)), 0);
			glGetProgramInfoLog(p, n, nullptr, log.data());
			throw std::runtime_error("Program link failed [" + label + "]: " + log);
		}

		return ShaderProgram(p);
	} catch (...) {
		if (vs)
			glDeleteShader(vs);
		if (fs)
			glDeleteShader(fs);
		if (p)
			glDeleteProgram(p);
		throw;
	}
}
} // namespace tanks::graphics
