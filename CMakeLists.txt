cmake_minimum_required(VERSION 3.8)
project(vstimgui)

# General configuration
OPTION(BUILD_STANDALONE "Build a standalone dummy version" ON)
set(VST2_SDK "empty" CACHE STRING "Path to Vst 2.4 sdk")
set(INCLUDED_FONT ${PROJECT_SOURCE_DIR}/imgui/misc/fonts/Roboto-Medium.ttf CACHE STRING "Path to font file to include in build")
set(OpenGL_GL_PREFERENCE "GLVND")
set(FONT_DIR ${CMAKE_BINARY_DIR}/generated)

find_package(OpenGL REQUIRED)

# The font used is compiled into the binary itself using the ImGui
# supplied tool to convert it to a header file.
file(MAKE_DIRECTORY ${FONT_DIR})
add_executable(binary_to_compressed EXCLUDE_FROM_ALL imgui/misc/fonts/binary_to_compressed_c.cpp)
add_custom_command(OUTPUT ${FONT_DIR}/font.h
                   COMMAND ./binary_to_compressed "${INCLUDED_FONT}" font > "${FONT_DIR}/font.h"
                   DEPENDS binary_to_compressed VERBATIM)

add_custom_target(generate_font DEPENDS ${FONT_DIR}/font.h)

# Build included glfw
set(BUILD_SHARED_LIBS       OFF CACHE BOOL "")
set(GLFW_BUILD_EXAMPLES     OFF CACHE BOOL "")
set(GLFW_BUILD_TESTS        OFF CACHE BOOL "")
set(GLFW_BUILD_DOCS         OFF CACHE BOOL "")
set(GLFW_INSTALL            OFF CACHE BOOL "")
set(GLFW_VULKAN_STATIC      OFF CACHE BOOL "")
add_subdirectory(glfw)

set(SOURCE_FILES src/editor.cpp)

set(IMGUI_FILES imgui/imgui.cpp
                imgui/imgui_draw.cpp
                imgui/imgui_widgets.cpp
                imgui/imgui_tables.cpp
                imgui/imgui_demo.cpp
                src/imgui_impl_glfw.cpp
                src/imgui_impl_opengl3.cpp
                imgui/examples/libs/gl3w/GL/gl3w.c)

set(EDITOR_LINK_LIBRARIES glfw ${OPENGL_LIBRARIES})

# Dear ImGui configuration
#  * Tell ImGui to put the global context in the thread local MyImGuiTLS variable
#  * Point to a local imconfig.h file with custom configuration
set(IMGUI_COMPILE_DEFINITIONS GImGui=MyImGuiTLS
                              IMGUI_USER_CONFIG=\"${PROJECT_SOURCE_DIR}/src/imconfig.h\")

set(EDITOR_LINK_LIBRARIES glfw ${OPENGL_LIBRARIES})
if(UNIX)
    set(EDITOR_COMPILE_OPTIONS -Wall -Wextra -Wno-psabi -ffast-math)
    set(EDITOR_COMPILE_DEFINITIONS PUBLIC ${EDITOR_COMPILE_DEFINITIONS} LINUX GLFW_EXPOSE_NATIVE_X11)
    set(EDITOR_LINK_LIBRARIES ${EDITOR_LINK_LIBRARIES} GLEW dl pthread)
elseif(MSVC)
    set(EDITOR_COMPILE_DEFINITIONS PUBLIC ${EDITOR_COMPILE_DEFINITIONS} WINDOWS GLFW_EXPOSE_NATIVE_WIN32)
endif()

add_library(vstimgui STATIC ${SOURCE_FILES} ${IMGUI_FILES})

target_compile_features(vstimgui PUBLIC cxx_std_20)
target_compile_definitions(vstimgui PRIVATE ${EDITOR_COMPILE_DEFINITIONS} ${IMGUI_COMPILE_DEFINITIONS})
target_compile_options(vstimgui PRIVATE ${EDITOR_COMPILE_OPTIONS})
add_dependencies(vstimgui generate_font)

target_include_directories(vstimgui PRIVATE imgui)
target_include_directories(vstimgui PRIVATE imgui/examples/libs/gl3w)
target_include_directories(vstimgui PRIVATE src)
target_include_directories(vstimgui PRIVATE ${FONT_DIR})
target_include_directories(vstimgui PUBLIC  include)

if (BUILD_STANDALONE)
    target_include_directories(vstimgui PUBLIC dummy_headers)
else()
    target_include_directories(vstimgui PUBLIC "${VST2_SDK}/public.sdk/source/vst2.x")
    target_include_directories(vstimgui PRIVATE "${VST2_SDK}")
endif()

target_link_libraries(vstimgui glfw  ${EDITOR_LINK_LIBRARIES})

if (BUILD_STANDALONE)
    add_executable(standalone_demo src/standalone_demo.cpp)
    target_link_libraries(standalone_demo vstimgui)
    if(UNIX)
        target_link_libraries(standalone_demo X11)
    endif()
endif()

