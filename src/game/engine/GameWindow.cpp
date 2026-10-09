#include "GameWindow.h"
#include "Settings.h"
#include <stdexcept>

namespace farmar::recovered {
namespace {
WindowTargets targets;
void active(bool active) {
    if (active) { if (targets.acquireInput) targets.acquireInput(); }
    else if (targets.releaseInput) targets.releaseInput();
    if (targets.pauseMain) targets.pauseMain(!active);
    if (targets.pauseSound) targets.pauseSound(!active);
}
}
WindowTargets& windowTargets() noexcept { return targets; }
LRESULT CALLBACK gameWindowProcedure(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    switch(message) {
    case WM_DESTROY: PostQuitMessage(0); return 0;
    case WM_ACTIVATEAPP:
        active(wparam==1);
        return DefWindowProcW(window,message,wparam,lparam);
    case WM_ACTIVATE:
        // Original tests the entire WPARAM, including its high word.
        if (wparam==0) { active(false); return 0; }
        if (wparam<3) { active(true); return 0; }
        break;
    case WM_MOUSEMOVE: return 0;
    case WM_CHAR:
        if (targets.character) targets.character(static_cast<std::uint32_t>(wparam));
        return 0;
    case WM_SYSCOMMAND:
        // Exact comparisons: original does not mask the low four command bits.
        if (wparam==SC_KEYMENU) return 0;
        if (targets.minimizeMain) {
            if (wparam==SC_RESTORE || wparam==SC_MAXIMIZE) targets.minimizeMain(false);
            if (wparam==SC_MINIMIZE) targets.minimizeMain(true);
        }
        break;
    }
    return DefWindowProcW(window,message,wparam,lparam);
}
HWND createGameWindow(HINSTANCE instance, Settings& settings, WString className) {
    WString title;
    settings.get(String("game"),String("window_title"),title);
    int width=0,height=0;
    settings.get(String("graphics"),String("width"),width);
    settings.get(String("graphics"),String("height"),height);
    if (className.empty()) className=title;
    WNDCLASSEXW klass{};
    klass.hIcon=LoadIconW(instance,MAKEINTRESOURCEW(101));
    if (!klass.hIcon) throw std::runtime_error("Cannot load game icon");
    klass.cbSize=48;
    klass.style=CS_CLASSDC;
    klass.lpfnWndProc=gameWindowProcedure;
    klass.hInstance=instance;
    klass.hCursor=LoadCursorW(nullptr,MAKEINTRESOURCEW(0x7f00));
    klass.hbrBackground=CreateSolidBrush(0);
    klass.lpszClassName=className.c_str();
    RegisterClassExW(&klass);
    // Original dimensions describe the outer window; no AdjustWindowRect.
    const auto window=CreateWindowExW(0,className.c_str(),title.c_str(),0x00ca0000,
        0,0,width,height,nullptr,nullptr,instance,nullptr);
    bool showCursor=false;
    settings.get(String("graphics"),String("ShowSysCursor"),showCursor);
    SetCursor(LoadCursorW(nullptr,showCursor?MAKEINTRESOURCEW(0x7f00):nullptr));
    DestroyIcon(klass.hIcon);
    return window;
}
void processGameMessages(const std::function<void()>& process) {
    MSG message;
    for (;;) {
        if (!PeekMessageW(&message,nullptr,0,0,PM_REMOVE)) { process(); continue; }
        if (message.message==WM_QUIT) return;
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
}
}
