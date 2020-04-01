#ifndef IMPLUGINGUI_CUSTOM_WIDGETS_H
#define IMPLUGINGUI_CUSTOM_WIDGETS_H

#include "imgui.h"
//#include "imgui_internal.h"

namespace ImGui
{
    IMGUI_API bool          VSliderFloatExt(const char* label, const ImVec2& size, float* v, float v_min, float v_max, const char* format = "%.3f", float power = 1.0f);
    IMGUI_API bool          VSliderIntExt(const char* label, const ImVec2& size, int* v, int v_min, int v_max, const char* format = "%d");
    IMGUI_API bool          VSliderScalarExt(const char* label, const ImVec2& size, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format = NULL, float power = 1.0f);

}

#endif //IMPLUGINGUI_CUSTOM_WIDGETS_H
