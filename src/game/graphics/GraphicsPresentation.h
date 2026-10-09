#pragma once
#include "Direct3D8.h"
#include <cstdint>
namespace farmar::recovered {
class Settings;
// Primary window/render properties from 0051a410. Lighting configuration is a
// separate unrecovered part of that function, not replaced by an empty backend.
struct PrimaryGraphicsSettings {
    int width=800,height=600;
    bool fullscreen=false,showTriangles=false,showFps=false,showCursorPosition=false;
    unsigned reservedSprites=1024;
    std::uint32_t background=0xff000000U;
    void load(Settings& settings);
};
struct DesktopImagePlacement {
    unsigned width=0,height=0;
    float scaleX=1,scaleY=1,offsetX=0,offsetY=0;
};
// Recovered presentation-parameter/window portion of cGraphics, not its whole
// 680-byte object or renderer. Device is supplied by the real Direct3D runtime.
class GraphicsPresentation {
public:
    GraphicsPresentation(IDirect3D8* runtime,HWND window); // portion of 0051c5d0
    void prepare(const PrimaryGraphicsSettings& settings); // 0051b9f0 after settings read
    const D3DPRESENT_PARAMETERS& parameters() const noexcept { return parameters_; }
    const D3DDISPLAYMODE& desktopMode() const noexcept { return desktop_; }
    const DesktopImagePlacement& imagePlacement() const noexcept { return placement_; }
    D3DFORMAT stencilFormat() const noexcept { return stencil_; }
private:
    IDirect3D8* runtime_;
    HWND window_;
    LONG savedStyle_;
    D3DDISPLAYMODE desktop_{};
    D3DFORMAT stencil_=D3DFMT_D24S8,fullscreenStencil_=D3DFMT_D24S8;
    DesktopImagePlacement placement_;
    D3DPRESENT_PARAMETERS parameters_{};
    D3DFORMAT chooseStencil(D3DFORMAT adapterFormat) const;
};
}
