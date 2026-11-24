cmake_minimum_required(VERSION 3.20)

project(IsometricTanks VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_package(OpenGL REQUIRED)

# Third-party support libraries are built from the bundled source.
set(FREEGLUT_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(FREEGLUT_BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
set(FREEGLUT_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
set(FREEGLUT_INSTALL_MAN_PAGES OFF CACHE BOOL "" FORCE)
add_subdirectory(dependencies/freeglut-3.8.0
	${CMAKE_BINARY_DIR}/third_party/freeglut EXCLUDE_FROM_ALL)

add_library(tanks_glew STATIC dependencies/glew-2.3.1/src/glew.c)
target_include_directories(tanks_glew SYSTEM PUBLIC
	dependencies/glew-2.3.1/include)
target_compile_definitions(tanks_glew PUBLIC GLEW_STATIC)
target_link_libraries(tanks_glew PUBLIC OpenGL::GL)

# Vendored Dear ImGui v1.93.0, with the existing FreeGLUT window backend.
add_library(tanks_imgui STATIC
	dependencies/imgui/imgui.cpp
	dependencies/imgui/imgui_draw.cpp
	dependencies/imgui/imgui_tables.cpp
	dependencies/imgui/imgui_widgets.cpp
	dependencies/imgui/backends/imgui_impl_glut.cpp
	dependencies/imgui/backends/imgui_impl_opengl3.cpp)
target_include_directories(tanks_imgui SYSTEM PUBLIC
	dependencies/imgui
	dependencies/imgui/backends)
target_include_directories(tanks_imgui SYSTEM PRIVATE
	dependencies/freeglut-3.8.0/include)
target_compile_definitions(tanks_imgui PRIVATE FREEGLUT_STATIC NOMINMAX)
target_link_libraries(tanks_imgui PUBLIC OpenGL::GL freeglut_static)
if(WIN32)
	target_link_libraries(tanks_imgui PRIVATE user32 shell32 imm32)
endif()

add_executable(tanks
	src/main.cpp

	src/core/math.cpp
	src/core/model_matrix.cpp
	src/core/object/game_object.cpp
	src/core/object/transform.cpp

	src/game/ai/ai.cpp
	src/game/combat/combat.cpp
	src/game/combat/effect.cpp
	src/game/combat/projectile.cpp
	src/game/combat/tank_health.cpp
	src/game/combat/tank_weapon.cpp
	src/game/input/keyboard.cpp
	src/game/input/player_input.cpp
	src/game/match/config.cpp
	src/game/match/match.cpp
	src/game/physics/collision.cpp
	src/game/physics/movement.cpp
	src/game/render/arena_model.cpp
	src/game/render/combat_model.cpp
	src/game/render/cover_model.cpp
	src/game/render/draw_context.cpp
	src/game/render/tank_model.cpp
	src/game/render/team_color.cpp
	src/game/tank.cpp
	src/game/world/arena.cpp
	src/game/world/directional_light.cpp
	src/game/world/world.cpp

	src/graphics/camera/camera.cpp
	src/graphics/camera/math.cpp
	src/graphics/renderer.cpp
	src/graphics/resources/mesh.cpp
	src/graphics/resources/shader.cpp
	src/graphics/resources/shadow_map.cpp
	src/graphics/scene/scene.cpp
	src/graphics/ui/hud.cpp

	src/app/cli/options.cpp
	src/app/input/input.cpp
	src/app/platform/imgui_input.cpp
	src/app/platform/window.cpp
	src/app/runtime/application.cpp
	src/app/runtime/callbacks.cpp
	src/app/runtime/timing.cpp)

target_include_directories(tanks PRIVATE src)
target_include_directories(tanks SYSTEM PRIVATE dependencies/freeglut-3.8.0/include)
target_link_libraries(tanks PRIVATE tanks_glew tanks_imgui freeglut_static)
target_compile_definitions(tanks PRIVATE FREEGLUT_STATIC NOMINMAX)
if(MINGW AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	target_link_options(tanks PRIVATE -static)
endif()
if(MSVC)
	target_compile_options(tanks PRIVATE /W4)
else()
	target_compile_options(tanks PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_custom_command(TARGET tanks POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${CMAKE_CURRENT_SOURCE_DIR}/src/shaders"
		"$<TARGET_FILE_DIR:tanks>/shaders"
	COMMENT "Copying file-loaded GLSL shaders")
