#include <thread>
#include <iostream>
#include <atomic>
#include <vector>
#include <csignal>

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
using NativeWindow = Window;
Window create_native_window(ERect* rect, Display* display)
{

    XSetWindowAttributes attributes = {0};
    attributes.event_mask = StructureNotifyMask;

    Window x11w = XCreateWindow(display, DefaultRootWindow(display), rect->left, rect->top,
                                rect->right, rect->bottom, 0, CopyFromParent, InputOutput,
                                CopyFromParent, 0, &attributes);

    Atom delete_atom = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, x11w, &delete_atom, 1);
    XSelectInput(display, x11w, StructureNotifyMask);

    XMapWindow(display, x11w);
    XSync(display, False);
    std::cout << "Created window " << (void*)x11w << std::endl;
    return x11w;
}
#endif
#ifdef WINDOWS
using NativeWindow = HWND;
using Display = int;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND create_native_window(ERect* rect, [[maybe_unused]]int display)
{
    const char CLASS_NAME[] = "ImGui plugin UI Demo";

    WNDCLASS wc = { };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    /* Copied from Microsoft example */
    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        CLASS_NAME,                     // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        rect->left, rect->top,
        rect->right, rect->bottom,

        nullptr,
        nullptr,
        GetModuleHandle(nullptr),  // Instance handle
        nullptr                    // Additional application data
        );

    if (!hwnd)
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
            running = false;
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

int main(int argc, char** argv)
{
    int n_windows = 1;
    if (argc > 1)
    {
        n_windows = std::atoi(argv[1]);
    }
    std::cout << "Using " << n_windows << " windows" << std::endl;

    signal(SIGINT, signal_handler);
    signal(SIGABRT, signal_handler);
    std::vector<std::pair<std::unique_ptr<AEffEditor>, NativeWindow>> editors(n_windows);

    /* Create a dummy plugin instance and pass to the Editor's factory function */
    AudioEffect plugin_dummy_instance;

    Display* display = nullptr;
#ifdef LINUX
    display = XOpenDisplay(nullptr);
#endif

    for (auto& editor : editors)
    {
        editor.first = imgui_editor::create_editor(&plugin_dummy_instance);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        ERect* rect = nullptr;
        /* Get the size of the Editor and create a system window to match this,
         * Essentially mimicking what a plugin host would do */
        editor.first->getRect(&rect);
        editor.second = create_native_window(rect, display);
        editor.first->open(reinterpret_cast<void*>(editor.second));
    }


    while(running)
    {
#ifdef LINUX
        /* Linux event handling */
        XEvent x_event;
        Atom close_msg = XInternAtom(display, "WM_DELETE_WINDOW", False);

        while (XPending(display))
        {
            XNextEvent(display, &x_event);
            if (x_event.type == ClientMessage && x_event.xclient.data.l[0] == close_msg)
            {
                Window window = x_event.xclient.window;
                /* Find the window whose close button was clicked and close it */
                for (auto editor = editors.begin(); editor != editors.end(); editor++)
                {
                    if (editor->second == window)
                    {
                        editor->first->close();
                        XDestroyWindow(display, window);
                        editors.erase(editor);
                        break;
                    }
                }
                if (editors.empty())
                {
                    running = false;
                }
            }
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
        for(auto& editor : editors)
        {
            editor.first->idle();
        }
    }

    /* If there are any windows still open, close them */
    for(auto& editor : editors)
    {
        editor.first->close();
#ifdef LINUX
        XDestroyWindow(display, editor.second);
#endif
    }

#ifdef LINUX
    XCloseDisplay(display);
#endif

    std::cout << "Bye!" << std::endl;
    return 0;
}
