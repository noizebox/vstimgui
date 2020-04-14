#include <thread>
#include <iostream>
#include <csignal>
#include <atomic>
#ifdef LINUX
#include <X11/Xlib.h>
#endif
#ifdef WINDOWS
#include <windows.h>
#endif

#include "aeffeditor.h"
#include "imgui_editor/imgui_editor.h"

std::atomic_bool running = true;

void signal_handler([[maybe_unused]] int sig_number)
{
    running = false;
}

#ifdef LINUX
Window create_native_window(ERect rect, Display* display)
{
    int blackColor = BlackPixel(display, DefaultScreen(display));
    Window x11w = XCreateSimpleWindow(display, DefaultRootWindow(display), rect.left, rect.top,
        rect.right, rect.bottom, 0, blackColor, blackColor);
    std::cout << "Window id: " << x11w << ", display: " << display << std::endl;
    XSync(display, False);

    auto res = XMapWindow(display, x11w);
    std::cout << "Map wind: " << res << std::endl;
    XSync(display, False);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    return x11w;
}
#endif
#ifdef WINDOWS
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND create_native_window(ERect rect)
{
    const char CLASS_NAME[] = "ImGui plugin UI Demo";

    WNDCLASS wc = { };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // Create the window.

    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        "Learn to Program Windows",     // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        rect.left, rect.top,
        rect.right, rect.bottom,

        NULL,       // Parent window    
        NULL,       // Menu
        GetModuleHandle(nullptr),  // Instance handle
        NULL        // Additional application data
        );

    if (hwnd == NULL)
    {
        return 0;
    }

    ShowWindow(hwnd, 5);
    return hwnd;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);



        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

        EndPaint(hwnd, &ps);
    }
    return 0;

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
#endif

int main(int, char**)
{
    signal(SIGINT, signal_handler);
    signal(SIGABRT, signal_handler);

    /* Create a dummy plugin instance and pass to the Editor's factory function */
    AudioEffect plugin_dummy_instance;
    auto editor = imgui_editor::create_editor(&plugin_dummy_instance);

    /* Get the size of the Editor and create a system window to match this,
     * Essentially mimicking what a plugin host would do */
    ERect rect;
    ERect* rect_ptr = &rect;
    editor->getRect(&rect_ptr);
    //auto b = get_val(5);

#ifdef LINUX
    auto display = XOpenDisplay(nullptr);
    auto native_win = create_native_window(rect, display);

    XEvent x_event;
#endif
#ifdef WINDOWS
    auto native_win = create_native_window(rect);
#endif

    editor->open(&native_win);

    while(running == true)
    {
#ifdef LINUX
        XNextEvent(display, &x_event);
        if (x_event.type == DestroyNotify)
        {
            running = false;
        }
#endif
#ifdef WINDOWS
        MSG msg = { };
        if (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
#endif
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        editor->idle();
    }
    editor->close();

#ifdef LINUX
    XCloseDisplay(display);
#endif
    std::cout << "Bye!" << std::endl;
    return 0;
}