## Vst 2.4 Plugin Editor with DearImGui
Work in progess. Example Open GL plugin editor using Dear ImGui with a glfw backend. It includes a small demo application for building it standalone with a dummy plugin, but can be included in an existing Vst2.4 plugin. Mostly intended as a demonstration of how one can setup a Vst Editor with Dear ImGui, as it's a quite attractive framework to use.

As the Vst2.4 SDK is deprecated and not supported by Steinberg anymore, it cannot be include it in the repo. To build an editor that will work with a real plugin and not just the supplied demo plugin, you'll need the real sdk. See the CMakeLists.txt file for how to add the path to it.

Currently the editor part is very limited and mostly intended as an example how to set things up. It will display sliders for up 10 parameters and some statistics on the draw time. 

### Building
Clone and initialise all submodules (or clone with the --recurse-submodules option), call cmake in a build dir and call 'make'.
Run 'standalone_demo' for an example.

### Including in a project
Include the repo folder using the cmake 'add_subdirectory' function, then link your target with 'implugingui' using 'target_link_libraries'. You also need to pass the path to your Vst SDK using the CMake variable VST2_SDK. CMake should handle the rest. 

### Dependencies not included
* Open GL 3
* glfw 3
* VstSDK 2.4

### Supported platforms
Currently Windows and Linux
