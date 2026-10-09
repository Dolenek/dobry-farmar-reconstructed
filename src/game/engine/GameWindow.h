#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "GameWString.h"
#include <functional>

namespace farmar::recovered {
class Settings;
// Call sites recovered from 00508c50. Bind to reconstructed input/main/sound
// objects as they are restored; empty functions represent original null globals.
struct WindowTargets {
    std::function<void()> acquireInput, releaseInput;
    std::function<void(std::uint32_t)> character;
    std::function<void(bool)> pauseMain, pauseSound, minimizeMain;
};
WindowTargets& windowTargets() noexcept;
LRESULT CALLBACK gameWindowProcedure(HWND window, UINT message, WPARAM wparam, LPARAM lparam); // 00508c50
HWND createGameWindow(HINSTANCE instance, Settings& settings, WString className); // 00509ab0
void processGameMessages(const std::function<void()>& process); // 005096a0
}
