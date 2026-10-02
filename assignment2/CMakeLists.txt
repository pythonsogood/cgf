cmake_minimum_required(VERSION 3.16)

project(assignment_2 LANGUAGES CXX)

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

function(add_assignment_target target_name)
	add_executable(${target_name} src/${target_name}.cpp)
	target_compile_definitions(${target_name} PRIVATE FREEGLUT_STATIC GLEW_STATIC)
	target_link_libraries(${target_name} PRIVATE GLUT::GLUT GLEW::GLEW OpenGL::GL
																							winmm gdi32)
endfunction()

add_assignment_target(animation)
add_assignment_target(cube)
