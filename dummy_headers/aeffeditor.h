#ifndef IMPLUGINGUI_DUMMY_VST_EDITOR_H
#define IMPLUGINGUI_DUMMY_VST_EDITOR_H

#include <cassert>
#include <array>
#include <string_view>

class AudioEffect
{
public:
    AudioEffect() = default;

    float getParameter(int index)
    {
        assert(index < _parameters.size());
        return _parameters[index].value;
    }

    void setParameterAutomated(int index, float value)
    {
        assert(index < _parameters.size());
        _parameters[index].value = value;
        std::cout << "Parameter " << _parameters[index].name << ": " << value << std::endl;
    }

    //void getParameterLabel(int index, char* label) = 0;
    //void getParameterDisplay(int index, char* text) = 0;
    void getParameterName(int index, char* text)
    {
        assert(index < _parameters.size());
        std::copy(_parameters[index].name.begin(), _parameters[index].name.end(), text);
    }

    static constexpr int PARAMETER_COUNT = 8;

private:
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
    AEffEditor (AudioEffect* effect = 0) : _effect(effect) {}

    virtual ~AEffEditor () = default;

    virtual bool getRect (ERect** rect) = 0;
    virtual bool open (void* window) = 0;
    virtual void close() = 0;
    virtual void idle() = 0;

protected:
    AudioEffect* _effect;
    void*        _main_window;
};

#endif //IMPLUGINGUI_DUMMY_VST_EDITOR_H