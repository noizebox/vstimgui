#ifndef IMPLUGINGUI_IMGUI_EDITOR_H
#define IMPLUGINGUI_IMGUI_EDITOR_H

#include <memory>

#include "aeffeditor.h"

namespace imgui_editor {

std::unique_ptr<AEffEditor> create_editor(AudioEffect* instance);

}
#endif //IMPLUGINGUI_IMGUI_EDITOR_H
