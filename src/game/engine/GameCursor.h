#pragma once
#include "GraphicsPresentation.h"
#include <array>
namespace farmar::recovered {
using CursorPoint=std::array<float,2>;
using CursorBounds=std::array<float,4>;
struct CursorServices {
    BOOL (WINAPI* windowInfo)(HWND,PWINDOWINFO)=&GetWindowInfo;
    BOOL (WINAPI* cursorPosition)(LPPOINT)=&GetCursorPos;
};
struct CursorSharedState { WINDOWINFO window{};POINT screen{}; };
CursorSharedState& cursorSharedState(); // 00660570 and 006605f4/f8
void cursorWindowBounds(HWND window,bool fullscreen,CursorBounds& output,CursorServices services={}); // 00512330
void cursorToGame(const CursorPoint& input,CursorPoint& output,bool fullscreen,const DesktopImagePlacement& placement) noexcept; // 0050d260
// Cursor component of cLogic (00530290 / getter 00531a40). The full cLogic
// singleton, frame machinery, shortcuts and original 24-byte ABI are separate.
class GameCursor {
public:
    explicit GameCursor(HWND window,CursorServices services={}):window_(window),services_(services) {}
    void update(bool fullscreen,const DesktopImagePlacement& placement); // 00530290
    const CursorPoint& position() const noexcept { return position_; } // 00531a40
private:
    HWND window_;CursorServices services_;CursorPoint position_{};
};
}
