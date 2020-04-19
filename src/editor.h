#ifndef IMPLUGINGUI_EDITOR_H
#define IMPLUGINGUI_EDITOR_H

#include <atomic>
#include <thread>
#include <cstdio>

#define NOMINMAX
#include "aeffeditor.h"
#include "imgui_editor/imgui_editor.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace imgui_editor {

//  About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
//  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
//  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>            // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)

#include <GL/glew.h>            // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>          // Initialize with gladLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/Binding.h>  // Initialize with glbinding::Binding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/glbinding.h>// Initialize with glbinding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

#ifdef LINUX
#define GLFW_EXPOSE_NATIVE_X11
#endif
#ifdef WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char*description)
{
    std::cerr << "Glfw Error " << error << ", " << description << std::endl;
}

class Editor : public AEffEditor
{
public:
    Editor(AudioEffect* instance);

    bool getRect(ERect**rect) override;

    bool open(void* window) override;

    void close() override;

    void idle() override;

private:
    bool _setup_open_gl(void* host_window);

    bool _setup_imgui();

    int _num_parameters;

    void _draw_loop(void* window);

    std::atomic_bool _running{false};
    std::thread _update_thread;
    ERect _rect;

#ifdef LINUX
    Window _host_window;
#elif WINDOWS
    HWND* _host_window;
#endif

    GLFWwindow*_window;
    float _slider_values[10];
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    ImVec2 slider_s{20, 105};
};

} // imgui_editor
#endif //IMPLUGINGUI_EDITOR_H
