#include <iostream>

#include "editor.h"
#include "custom_widgets.h"

namespace imgui_editor {

constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 400;
const char*glsl_version = "#version 130";

std::unique_ptr<AEffEditor> create_editor(AudioEffect* instance, int num_parameters)
{
    return std::make_unique<Editor>(instance, num_parameters);
}

Editor::Editor(AudioEffect* instance, int num_parameters) : AEffEditor::AEffEditor(instance),
                                                           _num_parameters(num_parameters)
{}

bool Editor::open(void* window)
{
    _setup_open_gl();
    _setup_imgui();
    while (!glfwWindowShouldClose(_window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();
        //glfwWaitEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        bool show_demo_window = false;
        bool show_another_window = true;

        /*
        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        //if (show_demo_window)
        //    ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        } */
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(WINDOW_WIDTH, WINDOW_HEIGHT));
        ImGui::SetNextWindowBgAlpha(0.0f);
        ImGui::Begin("__", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoMove);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
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
            std::cout << "Decay: " << _slider_values[1] << std::endl;
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
            std::cout << "Decay2: " << _slider_values[5] << std::endl;
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

        ImGuiDir dir = 0;
        ImGui::End();
        //}

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(_window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(_window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(_window);
    glfwTerminate();
}

void Editor::close()
{

}

bool Editor::_setup_open_gl()
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return false;

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
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    _window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Dear ImGui Plugin UI", NULL, NULL);
    if (_window == NULL)
        return 1;
    glfwMakeContextCurrent(_window);
    glfwSwapInterval(1); // Enable vsync

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
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
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
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    auto& style = ImGui::GetStyle();
    style.GrabRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.Colors[ImGuiCol_FrameBgHovered] = style.Colors[ImGuiCol_FrameBg];
    style.Colors[ImGuiCol_FrameBgActive] = style.Colors[ImGuiCol_FrameBg];
    style.Colors[ImGuiCol_SliderGrabActive] = style.Colors[ImGuiCol_SliderGrab];
    //style.ScaleAllSizes(2.0f);

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
    return false;
}

void Editor::idle()
{

}

} // imgui_editor