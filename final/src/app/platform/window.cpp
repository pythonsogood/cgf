#include <GL/glew.h>
#ifdef _WIN32
#include <GL/wglew.h>
#endif
#include <GL/freeglut.h>
#include "app/platform/window.hpp"
#include <stdexcept>

namespace tanks::app {
Window::Window(char **argv) {
	int argc = 1;

	glutInit(&argc, argv);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitWindowSize(1280, 720);

	id_ = glutCreateWindow("Isometric Tanks | Sultan Yakupov, Vladislav Kutsenko, "
	                       "Aibek Nurtay, Karima Shurenova");

	try {
		glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
		glewExperimental = GL_TRUE;

		const GLenum status = glewInit();
		if (status != GLEW_OK)
			throw std::runtime_error(
			    reinterpret_cast<const char *>(glewGetErrorString(status)));

		while (glGetError() != GL_NO_ERROR) {
		}

		if (!GLEW_VERSION_3_3)
			throw std::runtime_error("OpenGL 3.3 or newer is required");
#ifdef _WIN32
		if (WGLEW_EXT_swap_control)
			wglSwapIntervalEXT(0);
#endif

		GLint profile = 0;
		glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profile);

		if (!(profile & GL_CONTEXT_CORE_PROFILE_BIT))
			throw std::runtime_error("Driver did not provide a core context");
	} catch (...) {
		glutDestroyWindow(id_);
		id_ = 0;
		throw;
	}
}

Window::~Window() {
	if (id_)
		glutDestroyWindow(id_);
}
} // namespace tanks::app
