// Real recovered window/settings/presentation/buffer integration diagnostic.
// Does not claim a full game boot or complete cGraphics renderer.
#include "GraphicsPresentation.h"
#include "SpriteBuffers.h"
#include "GraphicsRenderState.h"
#include "GameWindow.h"
#include "FileIndex.h"
#include "ResourceManager.h"
#include "Settings.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include <cstring>
using namespace farmar::recovered;
namespace {
String path(const wchar_t* input) {
    int count=WideCharToMultiByte(CP_ACP,0,input,-1,nullptr,0,nullptr,nullptr);
    std::string output(count,'\0');
    WideCharToMultiByte(CP_ACP,0,input,-1,output.data(),count,nullptr,nullptr);
    return String(output.c_str());
}
}
int wmain(int argc,wchar_t** argv) {
    if(argc!=2) return 2;
    HMODULE runtime=nullptr;
    IDirect3D8* d3d=nullptr; IDirect3DDevice8* device=nullptr; HWND window=nullptr;
    unsigned checks=0;
    auto check=[&](bool result,const char* text) { if(!result) throw std::runtime_error(text); ++checks; };
    try {
        FileIndex files(path(argv[1]),String("pack")); ResourceManager manager(files);
        Settings settings(files,[&](const String& name) { return manager.loadSettingsXML(name); });
        PrimaryGraphicsSettings graphics; graphics.load(settings);
        check(graphics.width==800 && graphics.height==600 && graphics.fullscreen && graphics.reservedSprites==3000 && graphics.background==0,"original primary graphics settings");
        check(manager.cachedCount()==0,"settings XML remains cached");
        window=createGameWindow(GetModuleHandleW(nullptr),settings,WString(L"Recovered graphics diagnostic"));
        check(window!=nullptr,"game window failed");
        const LONG style=GetWindowLongW(window,GWL_STYLE);
        wchar_t system[MAX_PATH]; check(GetSystemDirectoryW(system,MAX_PATH)!=0,"system folder failed");
        const auto dll=std::wstring(system)+L"\\d3d8.dll";
        runtime=LoadLibraryW(dll.c_str()); check(runtime!=nullptr,"system Direct3D8 unavailable");
        using Factory=IDirect3D8*(WINAPI*)(UINT);
        const auto factory=reinterpret_cast<Factory>(GetProcAddress(runtime,"Direct3DCreate8"));
        check(factory!=nullptr,"Direct3DCreate8 missing"); d3d=factory(120); check(d3d!=nullptr,"SDK 120 failed");
        GraphicsPresentation presentation(d3d,window);
        check(presentation.stencilFormat()==D3DFMT_D24S8 || presentation.stencilFormat()==D3DFMT_D24X4S4,"stencil format differs");
        const auto& placement=presentation.imagePlacement(); const auto& desktop=presentation.desktopMode();
        check(placement.width<=desktop.Width && placement.height<=desktop.Height && placement.scaleX==placement.width/800.f && placement.scaleY==placement.height/600.f,"desktop image placement");
        graphics.fullscreen=false; // Diagnostic uses windowed mode; original data stays unchanged.
        presentation.prepare(graphics);
        RECT client{}; GetClientRect(window,&client);
        check(client.right==800 && client.bottom==600,"configured client dimensions");
        check(GetWindowLongW(window,GWL_STYLE)==(style|WS_VISIBLE),"window style differs");
        ShowWindow(window,SW_HIDE);
        auto parameters=presentation.parameters();
        check(parameters.Windowed && parameters.BackBufferWidth==800 && parameters.BackBufferHeight==600 && parameters.BackBufferCount==1 && parameters.SwapEffect==D3DSWAPEFFECT_DISCARD,"windowed parameters");
        check(parameters.EnableAutoDepthStencil && parameters.AutoDepthStencilFormat==presentation.stencilFormat() && parameters.MultiSampleType==D3DMULTISAMPLE_NONE && parameters.Flags==0 && parameters.FullScreen_RefreshRateInHz==0 && parameters.FullScreen_PresentationInterval==0,"windowed depth/interval flags");
        check(SUCCEEDED(d3d->CreateDevice(0,D3DDEVTYPE_HAL,window,D3DCREATE_SOFTWARE_VERTEXPROCESSING,&parameters,&device)),"original windowed device parameters failed");
        IDirect3DSurface8* depth=nullptr; check(SUCCEEDED(device->GetDepthStencilSurface(&depth)),"depth surface unavailable");
        D3DSURFACE_DESC description{}; depth->GetDesc(&description); depth->Release();
        check(description.Width==800 && description.Height==600 && description.Format==presentation.stencilFormat(),"depth surface description differs");
        {
            GraphicsRenderState render(device); render.setup(graphics,presentation);
            for(const auto& state:std::initializer_list<std::pair<D3DRENDERSTATETYPE,DWORD>>{
                {D3DRS_ALPHABLENDENABLE,1},{D3DRS_ALPHATESTENABLE,1},{D3DRS_SRCBLEND,5},{D3DRS_DESTBLEND,6},
                {D3DRS_ALPHAFUNC,5},{D3DRS_BLENDOP,1},{D3DRS_ALPHAREF,0},{D3DRS_LIGHTING,0},
                {D3DRS_CULLMODE,1},{D3DRS_ZENABLE,0},{D3DRS_ZWRITEENABLE,0}}) {
                DWORD actual=~0U; device->GetRenderState(state.first,&actual); check(actual==state.second,"recovered render state");
            }
            for(const auto& state:std::initializer_list<std::pair<D3DTEXTURESTAGESTATETYPE,DWORD>>{
                {D3DTSS_ALPHAOP,4},{D3DTSS_ALPHAARG1,0},{D3DTSS_ALPHAARG2,2},{D3DTSS_COLOROP,4},
                {D3DTSS_COLORARG1,0},{D3DTSS_COLORARG2,2},{D3DTSS_TEXCOORDINDEX,0},{D3DTSS_ADDRESSU,3},{D3DTSS_ADDRESSV,3}}) {
                DWORD actual=~0U; device->GetTextureStageState(0,state.first,&actual); check(actual==state.second,"recovered texture stage");
            }
            D3DMATRIX projection; device->GetTransform(D3DTS_PROJECTION,&projection);
            check(std::memcmp(&projection,&render.projection(),sizeof(Matrix4))==0,"native projection matrix differs");
            D3DMATERIAL8 material{}; device->GetMaterial(&material);
            check(material.Power==7 && material.Diffuse.r==1 && material.Emissive.a==1 && material.Specular.g==1,"original material default");
            // Exercise fullscreen state math/border allocation on this windowed
            // device without requesting a monitor mode change or fullscreen device.
            graphics.fullscreen=true; render.setup(graphics,presentation); graphics.fullscreen=false;
            check(render.borderVertices()!=nullptr,"fullscreen border allocation");
            D3DVERTEXBUFFER_DESC borders{}; render.borderVertices()->GetDesc(&borders);
            check(borders.Size==0x120 && borders.FVF==0x142 && borders.Usage==0x208,"border buffer description");
            SpriteVertex* black=nullptr;
            check(SUCCEEDED(render.borderVertices()->Lock(0,0x120,reinterpret_cast<BYTE**>(&black),0)),"border buffer lock");
            bool valid=true;
            for(unsigned i=0;i<12;++i) valid=valid && black[i].color==0xff000000U && black[i].z==0 && black[i].u==0 && black[i].v==0 &&
                black[i].x>=0 && black[i].x<=desktop.Width && black[i].y>=0 && black[i].y<=desktop.Height;
            render.borderVertices()->Unlock(); check(valid,"border vertex initialization/bounds");
            check(render.imageTransform().values[0]==placement.scaleX && render.imageTransform().values[5]==placement.scaleY &&
                render.imageTransform().values[12]==placement.offsetX && render.imageTransform().values[13]==placement.offsetY,"fullscreen image transform");
            render.setup(graphics,presentation);
            SpriteBuffers buffers(device);
            for(unsigned capacity:{graphics.reservedSprites,17U,graphics.reservedSprites}) {
                buffers.resize(capacity);
                check(buffers.indices() && buffers.vertices(),"original dynamic sprite buffers failed");
                D3DINDEXBUFFER_DESC index{}; D3DVERTEXBUFFER_DESC vertex{};
                buffers.indices()->GetDesc(&index); buffers.vertices()->GetDesc(&vertex);
                check(index.Size==capacity*12 && index.Format==D3DFMT_INDEX16 && index.Usage==0x208,"index buffer description");
                check(vertex.Size==capacity*96 && vertex.FVF==0x142 && vertex.Usage==0x208,"vertex buffer description");
                check(buffers.spriteCount()==capacity && buffers.indexCount()==capacity*6 && buffers.vertexCount()==capacity*4,"sprite counts");
            }
        }
        device->Release(); device=nullptr;
        graphics.fullscreen=true; presentation.prepare(graphics);
        const auto fullscreen=presentation.parameters();
        check(!fullscreen.Windowed && fullscreen.BackBufferWidth==desktop.Width && fullscreen.BackBufferHeight==desktop.Height && fullscreen.SwapEffect==static_cast<D3DSWAPEFFECT>(4),"fullscreen parameters");
        const DWORD fullscreenStyle=static_cast<DWORD>(GetWindowLongW(window,GWL_STYLE));
        // Windows adds WS_CLIPSIBLINGS after SetWindowLong(style=0). Check the
        // removed frame/visibility bits and retain the observed style in report.
        check(fullscreen.FullScreen_PresentationInterval==D3DPRESENT_INTERVAL_IMMEDIATE && fullscreen.Flags==0 &&
            (fullscreenStyle&~WS_CLIPSIBLINGS)==0,"fullscreen interval/style");
        graphics.fullscreen=false; presentation.prepare(graphics); ShowWindow(window,SW_HIDE);
        check(presentation.parameters().FullScreen_PresentationInterval==D3DPRESENT_INTERVAL_IMMEDIATE,"original interval retained across windowed switch");
        d3d->Release(); d3d=nullptr; DestroyWindow(window); window=nullptr; FreeLibrary(runtime); runtime=nullptr;
        std::cout << "{\"checks\":" << checks << ",\"fullscreen_observed_style\":" << fullscreenStyle
            << ",\"desktop_width\":" << desktop.Width << ",\"desktop_height\":" << desktop.Height << "}\n";
    } catch(const std::exception& error) {
        if(device) device->Release(); if(d3d) d3d->Release(); if(window) DestroyWindow(window); if(runtime) FreeLibrary(runtime);
        std::cerr << error.what() << '\n'; return 1;
    } catch(const char* error) { std::cerr << error << '\n'; return 1; }
}
