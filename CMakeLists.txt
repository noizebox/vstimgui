cmake_minimum_required(VERSION 3.8)
project(implugingui)

OPTION(BUILD_STANDALONE "Build a standalone dummy version" ON)
set(VST2_SDK "empty" CACHE STRING "Path to Vst 2.4 sdk")
set(OpenGL_GL_PREFERENCE "GLVND")

if(UNIX)
    find_package(PkgConfig REQUIRED)
    pkg_search_module(GLFW REQUIRED glfw3)
endif()
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

set(SOURCE_FILES src/editor.cpp)

set(IMGUI_FILES imgui/imgui.cpp
                imgui/imgui_draw.cpp
                imgui/imgui_widgets.cpp
                imgui/examples/imgui_impl_glfw.cpp
                imgui/examples/imgui_impl_opengl3.cpp
                imgui/examples/libs/gl3w/GL/gl3w.c)

set(EDITOR_LINK_LIBRARIES glfw ${OPENGL_LIBRARIES})
#set(EDITOR_COMPILE_DEFINITIONS -DIMGUI_IMPL_OPENGL_LOADER_GLAD=1)

if(UNIX)
    set(EDITOR_COMPILE_OPTIONS -Wall -Wextra -Wno-psabi -ffast-math)
    set(EDITOR_COMPILE_DEFINITIONS PUBLIC ${EDITOR_COMPILE_DEFINITIONS} -DLINUX)
    set(EDITOR_LINK_LIBRARIES ${EDITOR_LINK_LIBRARIES} GLEW dl pthread)
elseif(MSVC)
    set(EDITOR_COMPILE_DEFINITIONS PUBLIC ${EDITOR_COMPILE_DEFINITIONS} -DWINDOWS)
endif()

add_library(implugingui STATIC ${SOURCE_FILES} ${IMGUI_FILES})

target_compile_features(implugingui PUBLIC cxx_std_17)
target_compile_definitions(implugingui PRIVATE ${EDITOR_COMPILE_DEFINITIONS})
target_compile_options(implugingui PRIVATE ${EDITOR_COMPILE_OPTIONS})

target_include_directories(implugingui PRIVATE imgui)
target_include_directories(implugingui PRIVATE imgui/examples)
target_include_directories(implugingui PRIVATE imgui/examples/libs/gl3w)
target_include_directories(implugingui PRIVATE src)
target_include_directories(implugingui PUBLIC ${CMAKE_SOURCE_DIR}/include)

if (BUILD_STANDALONE)
    target_include_directories(implugingui PUBLIC dummy_headers)
else()
    target_include_directories(implugingui PRIVATE ${VST2_SDK})
endif()

target_link_libraries(implugingui glfw  ${EDITOR_LINK_LIBRARIES})

if (BUILD_STANDALONE)
    add_executable(standalone_demo src/standalone_demo.cpp)
    target_link_libraries(standalone_demo implugingui)
    if(UNIX)
        target_link_libraries(standalone_demo X11)
    endif()
endif()

