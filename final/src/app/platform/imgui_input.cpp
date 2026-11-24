#include <GL/glew.h>
#include <GL/freeglut.h>
#include "app/platform/imgui_input.hpp"
#include "imgui.h"
#include "backends/imgui_impl_glut.h"
#include "backends/imgui_impl_opengl3.h"
#include <stdexcept>

namespace tanks::app {
ImGuiInput::ImGuiInput() {
	if (!ImGui::GetCurrentContext())
		throw std::logic_error("ImGui platform backend requires an active context");
	if (!ImGui_ImplGLUT_Init())
		throw std::runtime_error("Could not initialize ImGui GLUT backend");
}

ImGuiInput::~ImGuiInput() { ImGui_ImplGLUT_Shutdown(); }
void ImGuiInput::beginFrame() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGLUT_NewFrame();
	ImGui::NewFrame();
}

void ImGuiInput::keyboard(unsigned char key, int x, int y) {
	ImGui_ImplGLUT_KeyboardFunc(key, x, y);
}

void ImGuiInput::keyboardUp(unsigned char key, int x, int y) {
	ImGui_ImplGLUT_KeyboardUpFunc(key, x, y);
}

void ImGuiInput::special(int key, int x, int y) {
	ImGui_ImplGLUT_SpecialFunc(key, x, y);
}

void ImGuiInput::specialUp(int key, int x, int y) {
	ImGui_ImplGLUT_SpecialUpFunc(key, x, y);
}

void ImGuiInput::mouseButton(int button, int state, int x, int y) {
	ImGui_ImplGLUT_MouseFunc(button, state, x, y);
}

void ImGuiInput::mouseMotion(int x, int y) { ImGui_ImplGLUT_MotionFunc(x, y); }

void ImGuiInput::mouseWheel(int button, int direction, int x, int y) {
	ImGui_ImplGLUT_MouseWheelFunc(button, direction, x, y);
}

void ImGuiInput::resize(int width, int height) {
	ImGui_ImplGLUT_ReshapeFunc(width, height);
}

bool ImGuiInput::captureMouse() const {
	return ImGui::GetIO().WantCaptureMouse;
}
bool ImGuiInput::captureKeyboard() const {
	return ImGui::GetIO().WantCaptureKeyboard;
}
} // namespace tanks::app
