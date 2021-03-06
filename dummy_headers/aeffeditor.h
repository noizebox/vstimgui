#ifndef IMPLUGINGUI_DUMMY_VST_EDITOR_H
#define IMPLUGINGUI_DUMMY_VST_EDITOR_H


/* This is a stand-in header just for building the demo in case the
 * Vst2.4 SDK is not available. As the SDK has been deprecated Steinberg,
 * it is no longer publicly available.
 * The intention of this header is not in any way to infringe on the
 * IP of Steinberg and it cannot be used to build a Vst Editor.
 *
 * The AudioEffect, ERect and AEffEditor classes defined in this header
 * only define small subset of the functions of the respective classes
 * in the Vst2.4 SDK and will not be ABI compatible with the AudioEffect
 * or AeffEditor classes defined there. */

#include <cassert>
#include <array>
#include <string_view>
#include <cstring>

struct AEffect
{
    int numParams;
};

class AudioEffect
{
public:
    AudioEffect() = default;

    AEffect* getAeffect ()
    {
        return &_effect;
    }

    float getParameter(int index)
    {
        assert(static_cast<size_t>(index) < _parameters.size());
        return _parameters[index].value;
    }

    void setParameterAutomated(int index, float value)
    {
        assert(static_cast<size_t>(index) < _parameters.size());
        _parameters[index].value = value;
        std::cout << "Parameter " << _parameters[index].name << ": " << value << std::endl;
    }

    void getParameterName(int index, char* text)
    {
        assert(static_cast<size_t>(index) < _parameters.size());
        memset(text, 0, 8);
        std::copy(_parameters[index].name.begin(), _parameters[index].name.end(), text);
    }

    static constexpr int PARAMETER_COUNT = 8;

private:
    AEffect  _effect{.numParams = PARAMETER_COUNT};

    struct Parameter
    {
        std::string_view name;
        float value;
    };

    std::array<Parameter, PARAMETER_COUNT> _parameters{
        Parameter{"Cutoff", 0.6f},
        Parameter{"Res", 0.3f},
        Parameter{"Attack", 0.0f},
        Parameter{"Decay", 0.2f},
        Parameter{"Sustain", 0.7f},
        Parameter{"Release", 0.2f},
        Parameter{"Gain", 1.0f},
        Parameter{"Pan", 0.5f}};
};

struct ERect
{
    int top;
    int left;
    int bottom;
    int right;
};

class AEffEditor
{
public:
    AEffEditor (AudioEffect* effect = nullptr) : effect(effect) {}

    virtual ~AEffEditor () = default;

    virtual bool getRect (ERect** rect) = 0;
    virtual bool open (void* window) = 0;
    virtual void close() = 0;
    virtual void idle() = 0;

protected:
    AudioEffect* effect;
    void*        _main_window;
};

#endif //IMPLUGINGUI_DUMMY_VST_EDITOR_H