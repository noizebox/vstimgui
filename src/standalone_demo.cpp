#include <thread>
#include <iostream>
#include <csignal>
#include <atomic>
#include <X11/Xlib.h>

#include "aeffeditor.h"
#include "imgui_editor/imgui_editor.h"

std::atomic_bool running = true;

void signal_handler([[maybe_unused]] int sig_number)
{
    running = false;
}

int main(int, char**)
{
    signal(SIGINT, signal_handler);

    /* Create a dummy plugin instance and pass to the Editor's factory function */
    AudioEffect plugin_dummy_instance;
    auto editor = imgui_editor::create_editor(&plugin_dummy_instance,
                                              AudioEffect::PARAMETER_COUNT);

    /* Get the size of the Editor and create a system window to match this,
     * Essentially mimicking what a plugin host would do */
    ERect rect;
    ERect* rect_ptr = &rect;
    editor->getRect(&rect_ptr);
    auto display = XOpenDisplay(nullptr);
    int blackColor = BlackPixel(display, DefaultScreen(display));
    Window x11w = XCreateSimpleWindow(display, DefaultRootWindow(display), rect.left, rect.top,
                                      rect.right, rect.bottom, 0, blackColor, blackColor);
    std::cout << "Window id: "<< x11w  << ", display: " << display << std::endl;
    XSync(display, False);

    auto res = XMapWindow(display, x11w);
    std::cout << "Map wind: " << res << std::endl;
    XSync(display, False);

    editor->open(&x11w);
    XEvent x_event;
    while(running == true)
    {
        XNextEvent(display, &x_event);
        if (x_event.type == DestroyNotify)
        {
            running = false;
        }
        //XPutBackEvent(display, &x_event);
        std::cout << "Xevent: " << x_event.type <<std::endl;
        //std::this_thread::sleep_for(std::chrono::milliseconds(50));
        editor->idle();
    }
    editor->close();
    std::cout << "Destroying windows" << std::endl;

    XCloseDisplay(display);
    std::cout << "Bye!" << std::endl;

    return 0;
}
