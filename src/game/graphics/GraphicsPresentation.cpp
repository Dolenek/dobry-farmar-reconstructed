#include "GraphicsPresentation.h"
#include "Settings.h"
#include <algorithm>
namespace farmar::recovered {
void PrimaryGraphicsSettings::load(Settings& settings) {
    const String section("graphics");
    settings.get(section,String("width"),width);
    settings.get(section,String("height"),height);
    settings.get(section,String("fullscreen"),fullscreen);
    settings.get(section,String("reserve_sprite_count"),reservedSprites);
    settings.get(section,String("show_triangles"),showTriangles);
    settings.get(section,String("show_fps"),showFps);
    settings.get(section,String("show_cursor_pos"),showCursorPosition);
    Color color; // original local four floats are zero before getProperty
    settings.get(section,String("back_color"),color);
    background=color.toARGB();
}
D3DFORMAT GraphicsPresentation::chooseStencil(D3DFORMAT adapterFormat) const {
    for(auto format:{D3DFMT_D24S8,D3DFMT_D24X4S4})
        if(SUCCEEDED(runtime_->CheckDeviceFormat(0,D3DDEVTYPE_HAL,adapterFormat,
            D3DUSAGE_DEPTHSTENCIL,D3DRTYPE_SURFACE,format))) return format;
    throw "Default video card dosn't support stensil buffer.";
}
GraphicsPresentation::GraphicsPresentation(IDirect3D8* runtime,HWND window)
    : runtime_(runtime),window_(window),savedStyle_(GetWindowLongW(window,GWL_STYLE)) {
    if(!runtime_) throw "Can't create Direct3D.";
    runtime_->GetAdapterDisplayMode(0,&desktop_);
    // Original constructor computes placement before loading settings, against
    // its fixed 800x600 defaults. Keep the original float constants and timing.
    const auto fitWidth=static_cast<int>(static_cast<double>(desktop_.Height)*1.33333337306976318359375);
    const auto fitHeight=static_cast<int>(static_cast<double>(desktop_.Width)*0.75);
    placement_.width=(std::min)(desktop_.Width,static_cast<unsigned>(fitWidth));
    placement_.height=(std::min)(desktop_.Height,static_cast<unsigned>(fitHeight));
    placement_.scaleX=static_cast<float>(placement_.width)/800.f;
    placement_.scaleY=static_cast<float>(placement_.height)/600.f;
    placement_.offsetX=static_cast<float>(desktop_.Width-placement_.width)*0.5f;
    placement_.offsetY=static_cast<float>(desktop_.Height-placement_.height)*0.5f;
    stencil_=fullscreenStencil_=chooseStencil(desktop_.Format);
}
void GraphicsPresentation::prepare(const PrimaryGraphicsSettings& settings) {
    parameters_.hDeviceWindow=window_;
    if(!settings.fullscreen) {
        D3DDISPLAYMODE current{};
        runtime_->GetAdapterDisplayMode(0,&current);
        stencil_=chooseStencil(current.Format);
        SetWindowLongW(window_,GWL_STYLE,savedStyle_);
        RECT rectangle{0,0,settings.width,settings.height};
        AdjustWindowRect(&rectangle,static_cast<DWORD>(savedStyle_),FALSE);
        // Subtraction and conversion are unsigned in the original instructions.
        const auto x=static_cast<int>(static_cast<double>(current.Width-static_cast<unsigned>(settings.width))*0.5);
        const auto y=static_cast<int>(static_cast<double>(current.Height-static_cast<unsigned>(settings.height))*0.5);
        SetWindowPos(window_,HWND_NOTOPMOST,x,y,rectangle.right-rectangle.left,rectangle.bottom-rectangle.top,
            SWP_NOACTIVATE|SWP_SHOWWINDOW);
        parameters_.BackBufferFormat=current.Format;
        parameters_.Windowed=TRUE;
        parameters_.SwapEffect=D3DSWAPEFFECT_DISCARD;
        parameters_.FullScreen_RefreshRateInHz=0;
        parameters_.BackBufferWidth=static_cast<unsigned>(settings.width);
        parameters_.BackBufferHeight=static_cast<unsigned>(settings.height);
    } else {
        SetWindowLongW(window_,GWL_STYLE,0);
        SetWindowPos(window_,HWND_TOPMOST,0,0,desktop_.Width,desktop_.Height,SWP_NOACTIVATE|SWP_NOREDRAW);
        parameters_.BackBufferFormat=desktop_.Format;
        stencil_=fullscreenStencil_;
        parameters_.Windowed=FALSE;
        parameters_.SwapEffect=static_cast<D3DSWAPEFFECT>(4); // original COPY_VSYNC value
        parameters_.FullScreen_RefreshRateInHz=0;
        parameters_.BackBufferWidth=desktop_.Width;
        parameters_.BackBufferHeight=desktop_.Height;
        parameters_.FullScreen_PresentationInterval=D3DPRESENT_INTERVAL_IMMEDIATE;
    }
    parameters_.AutoDepthStencilFormat=stencil_;
    parameters_.MultiSampleType=D3DMULTISAMPLE_NONE;
    parameters_.EnableAutoDepthStencil=TRUE;
    parameters_.BackBufferCount=1;
    // Original does not clear PresentationInterval when switching to windowed.
}
}
