#include <iostream>
#include <chrono>
#include <array>
#include <string>
#include <algorithm>
#include <iostream>

#include "editor.h"
#include "font.h"

#ifdef LINUX
#include <X11/Xlib.h>
#endif

thread_local ImGuiContext* MyImGuiTLS;

constexpr int WINDOW_WIDTH = 500;
constexpr int WINDOW_HEIGHT = 320;
constexpr int PARAM_SPACING = 50;
constexpr int MAX_PARAMETERS = 10;
constexpr int PING_INTERVALL = 300;
constexpr float SMOOTH_FACT = 0.05;
const char* glsl_version = "#version 130";

namespace imgui_editor {

std::mutex Editor::_init_lock;
std::atomic<int> Editor::instance_counter = 0;

#ifdef WINDOWS
void reparent_window(GLFWwindow* window, void* host_window)
{
    HWND hwnd = glfwGetWin32Window(window);
    if (SetParent(hwnd, reinterpret_cast<HWND>(host_window)) == nullptr)
    {
        std::cout << GetLastError() << std::endl;
    }
}

void reparent_window_to_root(GLFWwindow* window)
{
    HWND hWnd = glfwGetWin32Window(window);
    SetParent(hWnd, nullptr);
}
#endif

#ifdef LINUX
void reparent_window(GLFWwindow* window, void* host_window)
{
    Window host_x11_win = reinterpret_cast<Window>(host_window);
    auto display = glfwGetX11Display();
    auto glfw_x11_win = glfwGetX11Window(window);

    XReparentWindow(display, glfw_x11_win, host_x11_win, 0,0);// h_offset);
    glfwFocusWindow(window);
    XRaiseWindow(display, glfw_x11_win);
    XSync(display, true);
}

void reparent_window_to_root([[maybe_unused]]GLFWwindow* window) {}
#endif

std::unique_ptr<AEffEditor> create_editor(AudioEffect* instance)
{
    return std::make_unique<Editor>(instance);
}

Editor::Editor(AudioEffect* instance) : AEffEditor::AEffEditor(instance),
                                        _rect{0, 0, WINDOW_HEIGHT, WINDOW_WIDTH}
{
    _num_parameters = instance->getAeffect()->numParams;
}

bool Editor::open(void* window)
{
    _running = true;
    _update_thread = std::thread(&Editor::_draw_loop, this, window);
    return true;
}

void Editor::close()
{
    std::cout << "Closing window" << std::endl;

    if (_running)
    {
        reparent_window_to_root(_window);
        _running = false;
        if (_update_thread.joinable())
        {
            _update_thread.join();
        }
    }
}

bool Editor::_setup_open_gl(void* host_window)
{
    /* Belt and braces! This refcount is mostly for the imgui-glfw backend.
     * The refcounted initialization in the included glfw fork is useful
     * when there are multiple plugins (not just multiple instances) using
     * vstimgui */
    auto inst_no = instance_counter.fetch_add(1);
    if (inst_no == 0)
    {
        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit())
        {
            return false;
        }
        /* Until this feature is done in glfw https://github.com/glfw/glfw/issues/25
         * Open a glfw window and set it's native window a child of the host provided window */
    }

    // GL 3.0 + GLSL 130
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    _window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Dear ImGui Plugin UI", nullptr, nullptr);
    if (_window == nullptr)
    {
        std::cout << "Failed to create window"  << std::endl;
        return false;
    }

    reparent_window(_window, host_window);
    glfwMakeContextCurrent(_window);
    glfwSwapInterval(1);

    // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    bool err = gladLoadGL() == 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
    bool err = false;
    glbinding::Binding::initialize();
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
    bool err = false;
    glbinding::initialize([](const char* name) { return (glbinding::ProcAddress)glfwGetProcAddress(name); });
#else
    bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
    if (err)
    {
        std::cout << "Failed to initialize OpenGL loader!" << std::endl;
        return false;
    }
    return true;
}

bool Editor::_setup_imgui()
{
    /* Setup Dear ImGui context. To enable multiple, independent windows,
     * the context is thread local and each window has it's own context
     * and it's own rendering thread */

    IMGUI_CHECKVERSION();
    MyImGuiTLS = ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void) io;

    /* Setup Dear ImGui style */
    ImGui::StyleColorsDark();

    auto& style = ImGui::GetStyle();
    style.GrabRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.Colors[ImGuiCol_FrameBgHovered] = style.Colors[ImGuiCol_FrameBg];
    style.Colors[ImGuiCol_FrameBgActive] = style.Colors[ImGuiCol_FrameBg];
    style.Colors[ImGuiCol_SliderGrabActive] = style.Colors[ImGuiCol_SliderGrab];

    /* Setup Platform/Renderer backends */
    ImGui_ImplGlfw_InitForOpenGL(_window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    /* The font is loaded from generated/font.h. The font file is in generated by the
     * binary_to_source utility included in Dear ImGui, this util is built and run by
     * CMake when generating the make files. Default font is Roboto
     * To change font, set the CMake varible INCLUDED_FONT */
    ImFontConfig config;
    io.Fonts->AddFontFromMemoryCompressedTTF(font_compressed_data, font_compressed_size, 16 , &config);
    return true;
}

bool Editor::getRect(ERect** rect)
{
    *rect = &_rect;
    return true;
}

void Editor::idle()
{
    int param_count = std::min(_num_parameters, MAX_PARAMETERS);
    /* Quick way of updating parameter values. In a real plugin impl, you probably
     * only want to read parameter value that are "dirty" in the sense that they
     * have recently been changed from outside the editor */
    for (int i = 0; i < param_count; ++i)
    {
        _slider_values[i] = effect->getParameter(i);
    }
}

void Editor::_draw_loop(void* window)
{
    /* Setting up more than 1 context at the same time seems to be not 100% thread safe */
    {
        std::scoped_lock<std::mutex> lock(_init_lock);
        _setup_open_gl(window);
        _setup_imgui();
    }

    int count = 0;
    /* Only display a maximum of 10 parameters in this demo */
    int param_count = std::min(_num_parameters, MAX_PARAMETERS);

    /* It's somewhat against the philosophy of an immediate mode gui to
     * hold a separate state in the gui class, but I would still prefer
     * to mirror the parameter values here than polling at 60 Hz or
     * sharing a state with the dsp model.
     * It's just a demo anyway :) You can do as you please */

    for (int i = 0; i < param_count; ++i)
    {
        _slider_values[i] = effect->getParameter(i);
    }

    std::array<std::string, MAX_PARAMETERS> param_names;
    for (int i = 0; i < param_count; ++i)
    {
        char buffer[64];
        std::fill(buffer, buffer + 64, 0);
        effect->getParameterName(i, buffer);
        param_names[i] = buffer;
    }

    float draw_time = 0;
    float render_time = 0;
    float gl_render_time = 0;
    float swap_time = 0;
    while (!glfwWindowShouldClose(_window) && _running)
    {
        // Poll and handle events (inputs, window resize, etc.)
        glfwPollEvents();

        auto start_time = std::chrono::high_resolution_clock::now();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(WINDOW_WIDTH, WINDOW_HEIGHT));
        ImGui::SetNextWindowBgAlpha(0.0f);
        ImGui::Begin("__", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoMove);

        ImU32 colour = ImColor(0x41, 0x7c, 0x8c, 0xff);

        ImDrawList*draw_list = ImGui::GetWindowDrawList();
        draw_list->AddRectFilled(ImVec2(5, 5), ImVec2(param_count * PARAM_SPACING + 10, 180), colour, 3.0f, ImDrawCornerFlags_All);

        ImGui::Text("Parameters");
        ImGui::NewLine();

        /* Draw parameter labels */
        for (int i = 0; i < param_count; ++i)
        {
            ImGui::SameLine(8 + i * PARAM_SPACING, 10);
            ImGui::TextUnformatted(param_names[i].c_str());
        }
        ImGui::NewLine();

        /* Draw parameter sliders */
        for (int i = 0; i < param_count; ++i)
        {
            ImGui::SameLine(10 + i * PARAM_SPACING, 10);
            /* Hint, we're passing a format string of \"\" to keep ImGui
             * from printing the value inside the slider */
            ImGui::VSliderFloat(("##" + param_names[i]).c_str(), slider_s, &_slider_values[i], 0, 1.0f, "");
            if (ImGui::IsItemActive())
            {
                effect->setParameterAutomated(i, _slider_values[i]);
            }
        }
        ImGui::NewLine();

        /* Draw a value display */
        for (int i = 0; i < param_count; ++i)
        {
            ImGui::SameLine(7 + i * PARAM_SPACING, 10);
            ImGui::Text("%.2f", _slider_values[i]);
        }

        /* Finally show some statistics on cpu usage */
        ImGui::NewLine();
        ImGui::Text("Draw time: %.4f ms", draw_time);
        ImGui::Text("Render time: %.4f ms", render_time);
        ImGui::Text("Open GL render time: %.4f ms", gl_render_time);
        ImGui::Text("Swap time: %.4f ms", swap_time);
        ImGui::End();

        auto split_time = std::chrono::high_resolution_clock::now();

        /* Rendering */
        ImGui::Render();
        int display_w, display_h;
        auto split2_time = std::chrono::high_resolution_clock::now();

        glfwGetFramebufferSize(_window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        auto split3_time = std::chrono::high_resolution_clock::now();
        glfwSwapBuffers(_window);
        auto end_time = std::chrono::high_resolution_clock::now();
        if (count++ % PING_INTERVALL == 0)
        {
            std::cout << "Ping " << count / PING_INTERVALL << std::endl;
        }

        /* Filter the timings so they look a bit nicer */
        draw_time = (1.0f - SMOOTH_FACT) * draw_time + SMOOTH_FACT * (split_time - start_time).count() / 1'000'000.0f;
        render_time = (1.0f - SMOOTH_FACT) * render_time + SMOOTH_FACT * (split2_time - split_time).count() / 1'000'000.0f;
        gl_render_time = (1.0f - SMOOTH_FACT) * gl_render_time + SMOOTH_FACT * (split3_time - split2_time).count() / 1'000'000.0f;
        swap_time = (1.0f - SMOOTH_FACT) * swap_time + SMOOTH_FACT * (end_time - split3_time).count() / 1'000'000.0f;
    }
    /* Cleanup on exit */
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();
    ImGui_ImplGlfw_Shutdown();
    glfwDestroyWindow(_window);

    auto inst_no = instance_counter.fetch_add(-1);
    if (inst_no <= 1)
    {
        glfwTerminate();
    }
}

} // imgui_editor
