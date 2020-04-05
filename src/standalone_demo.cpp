#include <iostream>

#include "aeffeditor.h"
#include "imgui_editor/imgui_editor.h"

int main(int, char**)
{
    AudioEffect plugin_dummy_instance;
    auto editor = imgui_editor::create_editor(&plugin_dummy_instance, AudioEffect::PARAMETER_COUNT);
    editor->open(nullptr);

    return 0;
}
