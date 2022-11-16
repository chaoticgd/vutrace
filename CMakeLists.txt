cmake_minimum_required(VERSION 3.0)
project(vutrace)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	message(FATAL_ERROR "In-tree build detected. You should do an out-of-tree build instead:\n\tcmake -S . -B bin/")
endif()

include_directories(imgui)
add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLAD=1)
add_executable(vutrace
	vutrace.cpp
	imgui/imconfig.h
	imgui/imgui_demo.cpp
	imgui/imgui_draw.cpp
	imgui/imgui_widgets.cpp
	imgui/imgui.cpp
	imgui/examples/imgui_impl_glfw.cpp
	imgui/examples/imgui_impl_opengl3.cpp
	imgui/misc/cpp/imgui_stdlib.cpp
)

add_executable(vudis
	vudis.cpp
)

add_subdirectory(glad)
add_subdirectory(glfw)
target_link_libraries(vutrace glad glfw)
