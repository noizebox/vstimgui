cmake_minimum_required(VERSION 3.8)
project(implugingui)

#find_package(PkgConfig REQUIRED)
#pkg_search_module(GLFW REQUIRED glfw3)
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

set(SOURCE_FILES src/main.cpp
                 src/editor.cpp)

set(IMGUI_FILES imgui/imgui.cpp
                imgui/imgui_draw.cpp
                imgui/imgui_widgets.cpp
                imgui/examples/imgui_impl_glfw.cpp
                imgui/examples/imgui_impl_opengl3.cpp
                imgui/examples/libs/gl3w/GL/gl3w.c)


add_executable(implugingui "${SOURCE_FILES}" ${IMGUI_FILES})

target_compile_features(implugingui PRIVATE cxx_std_17)
target_compile_definitions(implugingui PRIVATE DIMGUI_IMPL_OPENGL_LOADER_GLAD=1)
#target_compile_options(implugingui PRIVATE -Wall -Wextra -Wno-psabi -ffast-math)

target_include_directories(implugingui PRIVATE imgui)
target_include_directories(implugingui PRIVATE imgui/examples)
target_include_directories(implugingui PRIVATE imgui/examples/libs/gl3w)
target_include_directories(implugingui PRIVATE src)

target_link_libraries(implugingui glfw  ${OPENGL_LIBRARIES})