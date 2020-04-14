#include <iostream>
#include <chrono>

#include "editor.h"
#include "custom_widgets.h"

#ifdef LINUX
#include <X11/Xlib.h>
#endif

namespace imgui_editor {

constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 400;
constexpr float SMOOTH_FACT = 0.05;
const char* glsl_version = "#version 130";

std::unique_ptr<AEffEditor> create_editor(AudioEffect* instance, int num_parameters)
{
    return std::make_unique<Editor>(instance, num_parameters);
}

Editor::Editor(AudioEffect* instance, int num_parameters) : AEffEditor::AEffEditor(instance),
                                                           _num_parameters(num_parameters),
                                                           _rect{0, 0, WINDOW_HEIGHT, WINDOW_WIDTH}
{}

bool Editor::open(void* window)
{
    _running = true;
    _update_thread = std::thread(&Editor::_draw_loop, this, window);
    return true;
}

void Editor::close()
{
    _running = false;
    if (_update_thread.joinable())
    {
        _update_thread.join();
    }
}

bool Editor::_setup_open_gl(void* host_window)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
    {
        return false;
    }
    /* Until this feature is done in glfw https://github.com/glfw/glfw/issues/25
     * Open a glfw window and set it's native window a child of the host provided window */

    // Decide GL+GLSL versions
#if __APPLE__
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
#endif
#ifdef WINDOWS
    /* On Linux we can't, for some reason, not reparent an undecorated window */
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
#endif

    _window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Dear ImGui Plugin UI", nullptr, nullptr);
    if (_window == nullptr)
    {
        std::cout << "Failed to create window"  << std::endl;
        return false;
    }

#ifdef LINUX
    Window root;
    Window parent;
    Window *child_list;
    unsigned int nch;
    XID host_win = *reinterpret_cast<Window*>(host_window);
    _host_window = host_win;
    auto display = glfwGetX11Display();
    auto x11_win = glfwGetX11Window(_window);

    /* The X11 window handle that we get from glfwGetX11Window() is in fact
     * wrapped in a container window and we can't reparent that directly.
     * We must get the parent of the given X11 window and reparent that to
     * the window provide by the host.
     * We also need to get the size of the parent window so we can get the
     * size of the title bar to move the window up in order to hide it
     * */
    XQueryTree(display,x11_win , &root, &parent, &child_list, &nch);
    XWindowAttributes attributes;
    int h_offset = 0;
    if (XGetWindowAttributes(display, parent, &attributes))
    {
        h_offset = _rect.bottom - attributes.height;
    }
    XReparentWindow(display, parent, _host_window, 0, h_offset);
#endif

#ifdef WINDOWS
    HWND hWnd = glfwGetWin32Window(_window);
    //SetWindowLongPtr(hWnd, GWL_STYLE, WS_POPUP | WS_CHILD);
    auto parent = GetParent(hWnd);
    //SetWindowLongPtr(hWnd, GWL_STYLE, (GetWindowLongPtr(hWnd, GWL_STYLE) & ~WS_POPUP) | WS_CHILD);
    if (SetParent(hWnd, *reinterpret_cast<HWND*>(host_window)))
    {
        _host_window = host_window;
    }
    else
    {
        std::cout << GetLastError() << std::endl;
    }
    //SetWindowPos(hWnd, HWND_TOP , 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, SWP_SHOWWINDOW);
 
#endif
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
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void) io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    auto& style = ImGui::GetStyle();
    style.GrabRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.Colors[ImGuiCol_FrameBgHovered] = style.Colors[ImGuiCol_FrameBg];
    style.Colors[ImGuiCol_FrameBgActive] = style.Colors[ImGuiCol_FrameBg];
    style.Colors[ImGuiCol_SliderGrabActive] = style.Colors[ImGuiCol_SliderGrab];

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(_window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFrdetaomFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    //    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    //    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    //    // - Read 'docs/FONTS.txt' for more instructions and ils.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    io.Fonts->AddFontFromFileTTF("../imgui/misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("c:/Users/Gustav/Programmering/dear-imgui-test-project/imgui/misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../imgui/misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("c:\users\Gustav\programmering\dear-imgui-test-project\ingui\misc\fonts\DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);
    return true;
}

bool Editor::getRect(ERect** rect)
{
    **rect = _rect;
    return true;
}

void Editor::idle()
{}

void Editor::_draw_loop(void* window)
{
    _setup_open_gl(window);
    _setup_imgui();
    int count = 0;

    float draw_time = 0;
    float render_time = 0;
    float gl_render_time = 0;
    float swap_time = 0;
    while (!glfwWindowShouldClose(_window) && _running)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
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
        //ImGui::BeginGroup();
        ImU32 colour = ImColor(0x41, 0x7c, 0x8c, 0xff);
        ImU32 colour2 = ImColor(0x61, 0x5c, 0x9c, 0xff);
        ImU32 colour_bg = ImColor(0x31, 0x5c, 0x6c, 0xff);
        ImU32 colour2_bg = ImColor(0x41, 0x4c, 0x7c, 0xff);
        ImDrawList*draw_list = ImGui::GetWindowDrawList();
        //draw_list->AddRectFilled(ImVec2(8, 8), ImVec2(183, 158), colour_bg,3.0f, ImDrawCornerFlags_All);
        draw_list->AddRectFilled(ImVec2(5, 5), ImVec2(180, 155), colour, 3.0f, ImDrawCornerFlags_All);
        //draw_list->AddRectFilled(ImVec2(8, 163), ImVec2(183, 318), colour2_bg,3.0f, ImDrawCornerFlags_All);
        draw_list->AddRectFilled(ImVec2(5, 160), ImVec2(180, 315), colour2, 3.0f, ImDrawCornerFlags_All);


        ImGui::Text("Filter ADSR");               // Display some text (you can use a format strings too)
        ImGui::NewLine();
        ImGui::SameLine(10, 10);

        ImGui::VSliderFloat("##1", slider_s, &_slider_values[0], 0, 10, 0);
        //std::cout << "Attack: " << _slider_values[0] << std::endl;
        ImGui::SameLine(50, 10);
        ImGui::VSliderFloat("##2", slider_s, &_slider_values[1], 0, 10, 0);
        if (ImGui::IsItemActive())
        {
            std::cout << "Decay: " << _slider_values[1] << std::endl;
        }
        ImGui::SameLine(90, 10);
        ImGui::VSliderFloat("##3", slider_s, &_slider_values[2], 0, 10, 0);
        ImGui::SameLine(130, 10);
        ImGui::VSliderFloat("##4", slider_s, &_slider_values[3], 0, 10, 0);
        //ImGui::EndGroup();
        ImGui::NewLine();
        ImGui::SameLine(10, 10);
        ImGui::TextUnformatted("Atk");
        ImGui::SameLine(60);
        ImGui::TextUnformatted("Dec");
        ImGui::SameLine(100);
        ImGui::TextUnformatted("Sus");
        ImGui::SameLine(140);
        ImGui::TextUnformatted("Rel");

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.0f);
        ImGui::Text("Amp ADSR");               // Display some text (you can use a format strings too)

        ImGui::NewLine();
        ImGui::SameLine(10, 10);

        ImGui::VSliderFloatExt("##5", slider_s, &_slider_values[4], 0, 10, 0);
        //std::cout << "Attack: " << _slider_values[0] << std::endl;
        ImGui::SameLine(50, 10);
        ImGui::VSliderFloatExt("##6", slider_s, &_slider_values[5], 0, 10, 0);
        if (ImGui::IsItemActive())
        {
            std::cout << "Decay2: " << _slider_values[5] << std::endl;
        }
        ImGui::SameLine(90, 10);
        ImGui::VSliderFloatExt("##7", slider_s, &_slider_values[6], 0, 10, 0);
        ImGui::SameLine(130, 10);
        ImGui::VSliderFloatExt("##8", slider_s, &_slider_values[7], 0, 10, 0);
        //ImGui::EndGroup();
        ImGui::NewLine();
        ImGui::SameLine(10, 10);
        ImGui::TextUnformatted("Atk");
        ImGui::SameLine(60);
        ImGui::TextUnformatted("Dec");
        ImGui::SameLine(100);
        ImGui::TextUnformatted("Sus");
        ImGui::SameLine(140);
        ImGui::TextUnformatted("Rel");

        ImGui::Text("Draw time: %f ms", draw_time);
        ImGui::Text("Render time: %f ms", render_time);
        ImGui::Text("Open GL render time: %f ms", gl_render_time);
        ImGui::Text("Swap time: %f ms", swap_time);
        ImGui::End();
        //}
        auto split_time = std::chrono::high_resolution_clock::now();

        // Rendering
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
        if (count++ % 60 == 0)
        {
            std::cout << "Ping " << count / 60 << std::endl;
        }

        /* Filter the timings so they look a bit nicer */
        draw_time = (1.0f - SMOOTH_FACT) * draw_time + SMOOTH_FACT * (split_time - start_time).count() / 1'000'000.0f;
        render_time = (1.0f - SMOOTH_FACT) * render_time + SMOOTH_FACT * (split2_time - split_time).count() / 1'000'000.0f;
        gl_render_time = (1.0f - SMOOTH_FACT) * gl_render_time + SMOOTH_FACT * (split3_time - split2_time).count() / 1'000'000.0f;
        swap_time = (1.0f - SMOOTH_FACT) * swap_time + SMOOTH_FACT * (end_time - split3_time).count() / 1'000'000.0f;
    }

    /* Cleanup on exit */
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(_window);
    glfwTerminate();
}

} // imgui_editor