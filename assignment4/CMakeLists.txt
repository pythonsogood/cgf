cmake_minimum_required(VERSION 3.16)

project(assignment_4 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_PREFIX_PATH
					"${CMAKE_CURRENT_SOURCE_DIR}/dependencies/freeglut-3.8.0")
set(FREEGLUT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/freeglut-3.8.0")
set(GLEW_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glew-2.3.1")
set(ENV{GLEW_ROOT} "${GLEW_ROOT}")

find_package(OpenGL REQUIRED)

set(GLEW_USE_STATIC_LIBS TRUE)
find_package(GLEW REQUIRED)

set(GLUT_ROOT_PATH "${FREEGLUT_ROOT}")
set(OPENGL_LIBRARY_DIR "${FREEGLUT_ROOT}/build/lib")
set(GLUT_glut_LIBRARY_RELEASE "${FREEGLUT_ROOT}/build/lib/libfreeglut_static.a"
				CACHE FILEPATH "Static freeglut library" FORCE)
find_package(GLUT REQUIRED)

set(targets viewer picking texture2d texture3d)
foreach(target IN LISTS targets)
	add_executable(${target} src/${target}.cpp)
	target_compile_features(${target} PRIVATE cxx_std_17)
	target_compile_definitions(${target} PRIVATE FREEGLUT_STATIC GLEW_STATIC)
	target_link_libraries(${target} PRIVATE GLUT::GLUT GLEW::GLEW OpenGL::GL
		winmm gdi32)
	set_property(TARGET ${target} APPEND PROPERTY LINK_DEPENDS
		"${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/viewer.vert"
		"${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/viewer.frag")
	add_custom_command(TARGET ${target} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
			"${CMAKE_CURRENT_SOURCE_DIR}/src/shaders"
			"$<TARGET_FILE_DIR:${target}>/shaders")
endforeach()
