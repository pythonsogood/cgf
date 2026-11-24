#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/freeglut_ucall.h>
#include "app/runtime/application.hpp"

namespace tanks::app {
void Application::registerCallbacks() {
	glutDisplayFuncUcall(displayCallback, this);
	glutIdleFunc(glutPostRedisplay);
	glutReshapeFuncUcall(reshapeCallback, this);
	glutKeyboardFuncUcall(keyboardCallback, this);
	glutKeyboardUpFuncUcall(keyboardUpCallback, this);
	glutSpecialFuncUcall(specialCallback, this);
	glutSpecialUpFuncUcall(specialUpCallback, this);
	glutMouseFuncUcall(mouseCallback, this);
	glutMotionFuncUcall(motionCallback, this);
	glutPassiveMotionFuncUcall(motionCallback, this);
	glutMouseWheelFuncUcall(mouseWheelCallback, this);
	glutEntryFuncUcall(entryCallback, this);
	glutCloseFuncUcall(closeCallback, this);
}

void Application::displayCallback(void *user) {
	static_cast<Application *>(user)->onDisplay();
}

void Application::reshapeCallback(int width, int height, void *user) {
	static_cast<Application *>(user)->onResize(width, height);
}

void Application::keyboardCallback(unsigned char key, int x, int y,
                                   void *user) {
	auto *app = static_cast<Application *>(user);
	app->imguiInput_->keyboard(key, x, y);
	app->onKey(key);
}

void Application::keyboardUpCallback(unsigned char key, int x, int y,
                                     void *user) {
	auto *app = static_cast<Application *>(user);
	app->imguiInput_->keyboardUp(key, x, y);
	app->input_.keyUp(key);
}

void Application::specialCallback(int key, int x, int y, void *user) {
	auto *app = static_cast<Application *>(user);
	app->imguiInput_->special(key, x, y);
	app->onSpecial(key);
}

void Application::specialUpCallback(int key, int x, int y, void *user) {
	auto *app = static_cast<Application *>(user);
	app->imguiInput_->specialUp(key, x, y);
	app->input_.specialUp(key);
}

void Application::mouseCallback(int button, int state, int x, int y,
                                void *user) {
	auto *app = static_cast<Application *>(user);
	app->imguiInput_->mouseButton(button, state, x, y);
	app->input_.mouseButton(button, state, x, y);
}

void Application::motionCallback(int x, int y, void *user) {
	auto *app = static_cast<Application *>(user);
	app->imguiInput_->mouseMotion(x, y);
	app->onMotion(x, y);
}

void Application::mouseWheelCallback(int button, int direction, int x, int y,
                                     void *user) {
	auto *app = static_cast<Application *>(user);
	app->imguiInput_->mouseWheel(button, direction, x, y);
	app->cameraInput_.zoom += float(direction);
}

void Application::entryCallback(int state, void *user) {
	if (state == GLUT_LEFT)
		static_cast<Application *>(user)->input_.releaseAll();
}

void Application::closeCallback(void *user) {
	static_cast<Application *>(user)->onClose();
}
} // namespace tanks::app
