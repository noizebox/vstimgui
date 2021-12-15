## Vst 2.4 Plugin Editor with DearImGui
Work in progress. Example VST 2.4 OpenGL editor using Dear ImGui with a glfw backend. It includes a small demo application for building it standalone with a dummy plugin, but can be included in an existing Vst2.4 plugin. Mostly intended as a demonstration of how one can setup a Vst Editor with Dear ImGui, as it's an attractive framework to use for plugin guis.

As the Vst2.4 SDK is deprecated and not supported by Steinberg anymore, it is not included in the repo. To build an editor that will work with a real plugin and not just the supplied demo plugin, you'll need the real sdk. See the CMakeLists.txt file for how to add the path to it.

VstImGui uses thread local data and a separate processing/draw thread per window to support multiple instances. There is also an included fork of glfw that adds some features neccesary to work in a child window and with multiple instances of the same window.

The editor part is very limited and mostly intended as an example how to set things up. It will display sliders for up 10 parameters and some statistics on the draw time.

### Building
Clone and initialise all submodules (or clone with the _--recurse-submodules_ option), call cmake in a build dir and call _make_.
Run _standalone_demo_ for an example.

### Including in a project
Include the vstimgui folder using the cmake _add_subdirectory_ function, then link your target with  _vstimgui_ using _target_link_libraries_. You also need to pass the path to the Vst SDK using the CMake variable VST2_SDK. CMake should handle the rest.

### Dependencies not included
* OpenGL 3
* VstSDK 2.4 (dummy headers for demo app included)

### Supported platforms
Currently Windows and Linux
