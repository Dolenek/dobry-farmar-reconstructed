#include "GameCursor.h"
#include "OriginalArithmetic.h"
namespace farmar::recovered {
CursorSharedState& cursorSharedState() { static CursorSharedState state;return state; }
void cursorWindowBounds(HWND window,bool fullscreen,CursorBounds& output,CursorServices services) {
    auto& state=cursorSharedState();
    // The original shared WINDOWINFO lies in zero-initialized .bss. This
    // function does not set cbSize; preserve exactly what the API receives.
    if(services.windowInfo(window,&state.window)) {
        const auto& bounds=fullscreen ? state.window.rcWindow:state.window.rcClient;
        output={float(bounds.left),float(bounds.top),float(bounds.right),float(bounds.bottom)};
    }
}
void cursorToGame(const CursorPoint& input,CursorPoint& output,bool fullscreen,const DesktopImagePlacement& placement) noexcept {
    // Copy both components before transforming: original permits aliasing.
    output=input;
    if(fullscreen) {
        output[0]=static_cast<float>(originalSubtract(output[0],placement.offsetX));
        output[1]=static_cast<float>(originalSubtract(output[1],placement.offsetY));
        output[0]=static_cast<float>(originalDivide(output[0],placement.scaleX));
        output[1]=static_cast<float>(originalDivide(output[1],placement.scaleY));
    }
}
void GameCursor::update(bool fullscreen,const DesktopImagePlacement& placement) {
    CursorBounds bounds{};cursorWindowBounds(window_,fullscreen,bounds,services_);
    auto& state=cursorSharedState();
    if(services_.cursorPosition(&state.screen)) {
        position_[0]=static_cast<float>(originalSubtract(double(state.screen.x),bounds[0]));
        position_[1]=static_cast<float>(originalSubtract(double(state.screen.y),bounds[1]));
        cursorToGame(position_,position_,fullscreen,placement);
    }
}
}
