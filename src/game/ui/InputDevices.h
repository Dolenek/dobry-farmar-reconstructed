#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include <windows.h>
#include <dinput.h>
#include "InputBindings.h"
#include <array>
namespace farmar::recovered {
// DirectInput device/polling part of cInput. Key-name map/old object ABI remain
// separate work; this class owns the actual devices and recovered binding sets.
class InputDevices {
public:
    InputDevices(HINSTANCE instance,HWND window); // device portion of 0052ca10
    InputDevices(HWND window,IDirectInputDevice8A* keyboard,IDirectInputDevice8A* mouse) noexcept; // diagnostic owned-device boundary
    ~InputDevices(); // release portion of 0052c530
    InputDevices(const InputDevices&)=delete;
    InputDevices& operator=(const InputDevices&)=delete;
    InputBindings& bindings() noexcept { return bindings_; }
    bool acquire(); // 00523e20
    bool unacquire(); // 00524200
    void exclusiveMouse(bool value); // 00525a00
    void character(unsigned value) noexcept; // 005245e0, max 1024 UTF-16 units
    bool keyPressed(unsigned control); // 00525620 / 00525240
    void process(); // 00527310
    void bindWindowTargets(); // recovered window singleton call sites
    unsigned queuedCharacters() const noexcept { return characterCount_; }
private:
    HWND window_;
    IDirectInputDevice8A *keyboard_=nullptr,*mouse_=nullptr;
    InputBindings bindings_;
    std::array<wchar_t,1024> characters_{};
    unsigned characterCount_=0;
    std::array<unsigned char,256> keyboardState_{};
    bool keyboardStateValid_=false,exclusive_=false;
};
}
