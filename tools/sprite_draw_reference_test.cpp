// Original 00515830 is used exclusively as a test oracle with an owned native
// device and fixture list. No original entry point/import initialization runs.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "FileIndex.h"
#include "ResourceManager.h"
#include "Texture.h"
#include "Shader.h"
#include "GraphicsPresentation.h"
#include "GraphicsRenderState.h"
#include "SpriteBatches.h"
#include "LetterSprite.h"
#include "StringManager.h"
#include "Settings.h"
#include "XmlDocument.h"
#include "ImageTexture.h"
#include "sprite_device_trace.h"
#include <array>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
using namespace farmar::recovered;
namespace {
void require(bool success,const char* message) { if(!success) throw std::runtime_error(message); }
String path(const wchar_t* wide) {
    const auto count=WideCharToMultiByte(CP_ACP,0,wide,-1,nullptr,0,nullptr,nullptr);
    std::vector<char> bytes(count); WideCharToMultiByte(CP_ACP,0,wide,-1,bytes.data(),count,nullptr,nullptr);
    return String(bytes.data());
}
struct Fixup { DWORD rva,address; };
const Fixup fixups[]={
#include "sprite_draw_oracle_fixups.inc"
};
template<class Value> void put(std::array<unsigned char,680>& object,unsigned offset,const Value& value) {
    std::memcpy(object.data()+offset,&value,sizeof(value));
}
struct Node { Node* next; Node* previous; DWORD words[9]; };
struct ConstantNode { ConstantNode* next; ConstantNode* previous; DWORD index; const float* data; DWORD count; };
struct ConstantList { DWORD allocator=0; ConstantNode* head; DWORD count; };
static_assert(sizeof(Node)==44,"Original graphics batch node");
std::vector<unsigned char> pixels(IDirect3DDevice8* device) {
    IDirect3DSurface8* back=nullptr; require(SUCCEEDED(device->GetBackBuffer(0,D3DBACKBUFFER_TYPE_MONO,&back)),"backbuffer unavailable");
    D3DSURFACE_DESC description{}; back->GetDesc(&description);
    IDirect3DSurface8* image=nullptr;
    const auto created=device->CreateImageSurface(description.Width,description.Height,description.Format,&image);
    if(FAILED(created)) { back->Release(); throw std::runtime_error("readback image unavailable"); }
    const auto copied=device->CopyRects(back,nullptr,0,image,nullptr); back->Release();
    if(FAILED(copied)) { image->Release(); throw std::runtime_error("backbuffer readback failed"); }
    D3DLOCKED_RECT locked{}; require(SUCCEEDED(image->LockRect(&locked,nullptr,D3DLOCK_READONLY)),"readback lock failed");
    require(description.Format==D3DFMT_X8R8G8B8 || description.Format==D3DFMT_A8R8G8B8,"readback format not 32-bit");
    std::vector<unsigned char> result(description.Width*description.Height*4);
    for(unsigned y=0;y<description.Height;++y) std::memcpy(result.data()+y*description.Width*4,
        static_cast<unsigned char*>(locked.pBits)+y*locked.Pitch,description.Width*4);
    // X8R8G8B8 has no defined alpha channel. Mask only its unused high byte;
    // A8R8G8B8 retains all four channels for comparison.
    if(description.Format==D3DFMT_X8R8G8B8) for(unsigned i=3;i<result.size();i+=4) result[i]=0;
    image->UnlockRect(); image->Release(); return result;
}
SpriteDefinition quad(float x,float y,float w,float h,DWORD color,unsigned subdivisions=1) {
    SpriteDefinition sprite{}; sprite.subdivisions=subdivisions;
    sprite.corners[0]={x,y,color}; sprite.corners[1]={x+w,y,color}; sprite.corners[2]={x,y+h,color}; sprite.corners[3]={x+w,y+h,color};
    sprite.u1=1; sprite.v1=1; return sprite;
}
}
int wmain(int argc,wchar_t** argv) {
    if(argc!=4) return 2;
    HMODULE runtime=nullptr,reference=nullptr; HWND window=nullptr; IDirect3D8* d3d=nullptr; IDirect3DDevice8* device=nullptr;
    try {
        reference=LoadLibraryExW(argv[1],nullptr,DONT_RESOLVE_DLL_REFERENCES); require(reference!=nullptr,"original oracle mapping failed");
        auto* original=reinterpret_cast<unsigned char*>(reference);
        for(const auto& fixup:fixups) {
            DWORD value; std::memcpy(&value,original+fixup.rva,4); require(value==fixup.address,"original operand differs");
            DWORD protection; require(VirtualProtect(original+fixup.rva,4,PAGE_EXECUTE_READWRITE,&protection)!=0,"oracle protection failed");
            value=reinterpret_cast<DWORD>(original)+(value-0x400000U); std::memcpy(original+fixup.rva,&value,4);
            DWORD ignored; VirtualProtect(original+fixup.rva,4,protection,&ignored);
        }
        FlushInstructionCache(GetCurrentProcess(),original+0x115830,0x1046);
        using Draw=void(__thiscall*)(void*); const auto draw=reinterpret_cast<Draw>(original+0x115830);
        WNDCLASSW cls{}; cls.lpfnWndProc=DefWindowProcW; cls.hInstance=GetModuleHandleW(nullptr); cls.lpszClassName=L"Recovered sprite draw oracle";
        require(RegisterClassW(&cls)!=0,"diagnostic class failed");
        window=CreateWindowW(cls.lpszClassName,cls.lpszClassName,WS_OVERLAPPEDWINDOW,0,0,800,600,nullptr,nullptr,cls.hInstance,nullptr);
        require(window!=nullptr,"diagnostic window failed");
        wchar_t system[MAX_PATH]; GetSystemDirectoryW(system,MAX_PATH); runtime=LoadLibraryW((std::wstring(system)+L"\\d3d8.dll").c_str());
        require(runtime!=nullptr,"native Direct3D8 unavailable");
        using Factory=IDirect3D8*(WINAPI*)(UINT); auto factory=reinterpret_cast<Factory>(GetProcAddress(runtime,"Direct3DCreate8"));
        require(factory!=nullptr,"Direct3D factory missing"); d3d=factory(120); require(d3d!=nullptr,"SDK 120 failed");
        PrimaryGraphicsSettings settings; settings.width=800; settings.height=600; settings.fullscreen=false;
        GraphicsPresentation presentation(d3d,window); presentation.prepare(settings); ShowWindow(window,SW_HIDE);
        auto parameters=presentation.parameters();
        require(SUCCEEDED(d3d->CreateDevice(0,D3DDEVTYPE_HAL,window,D3DCREATE_SOFTWARE_VERTEXPROCESSING,&parameters,&device)),"native HAL device failed");
        unsigned scenarios=0; std::uint64_t comparedPixels=0,drawnPixels=0;
        {
            FileIndex files(path(argv[2]),String("pack")); files.mountCommon(path(argv[3])); ResourceManager manager(files);
            textureEnvironment().files=&files;
            textureEnvironment().load=[&](const void* bytes,const unsigned long& size,IDirect3DTexture8*& texture,TextureImageSize& dimensions) { getImageTexture(device,bytes,size,texture,dimensions); };
            cTexture* textures[]={manager.getTexture(String("UI/MainMenu/log-yes.jng")),manager.getTexture(String("UI/MainMenu/small_button2.jng"))};
            shaderEnvironment().files=&files;
            shaderEnvironment().create=[&](const DWORD* code,const unsigned long&,DWORD& handle) { createSpriteVertexShader(device,code,handle); };
            shaderEnvironment().remove=[&](const DWORD& handle) { device->DeleteVertexShader(handle); };
            auto* shader=manager.getShader(String("sprite-fixture.vs")); require(shader->shader()!=0,"fixture vertex shader failed");
            require(manager.getShader(String("SPRITE-FIXTURE.VS"))==shader && shader->referenceCount()==2,"shader resource cache/refcount differs");
            shader->releaseReference();
            shader->preResetGraphics(); require(shader->shader()==0,"shader reset did not zero handle");
            shader->restoreGraphics(); require(shader->shader()!=0,"shader restore failed");
            const float tint[]={0.75f,0.875f,1.f,0.8f},unused[]={0.1f,0.2f,0.3f,0.4f};
            ShaderConstants constants; constants.values={{4,tint,1},{5,unused,1}};
            ConstantNode constantNodes[3]{};
            for(unsigned i=0;i<3;++i) { constantNodes[i].next=&constantNodes[(i+1)%3]; constantNodes[i].previous=&constantNodes[(i+2)%3]; }
            constantNodes[1].index=4; constantNodes[1].data=tint; constantNodes[1].count=1;
            constantNodes[2].index=5; constantNodes[2].data=unused; constantNodes[2].count=1;
            ConstantList originalConstants{0,&constantNodes[0],2};
            SpriteBuffers buffers(device); buffers.resize(3000); SpriteBatches sprites(buffers); GraphicsRenderState render(device);
            fontFiles()=&files;
            cFont* fonts[]={manager.getFont(String("UI/Font/font_tips.fnt")),manager.getFont(String("UI/honey_font.fnt")),manager.getFont(String("UI/menu_font.fnt"))};
            cTexture* fontTextures[]={manager.getTexture(String("UI/Font/font_tips.jng")),manager.getTexture(String("UI/honey_font.jng")),manager.getTexture(String("UI/menu_font.dds"))};
            Settings dataSettings(files,[&](const String& name) { return manager.loadSettingsXML(name); });
            StringManager strings(files,dataSettings,manager);
            std::vector<WString> menuLabels;
            {
                auto menu=manager.loadSettingsXML(String("UI/MainMenu.xml"));
                for(const char* name:{"StartButton","CreditsButton","Options","ExitButton","ChangeUserButton","UserText"}) {
                    XmlNode* node=nullptr; require(menu->section(String(name),node),"Original menu label missing");
                    XmlNode* label=nullptr; if(node->section(String("Text"),label)) node=label;
                    auto text=node->attribute(WString(L"String")); menuLabels.push_back(strings.getString(text));
                }
            }
            for(unsigned trial=0;trial<108;++trial) {
                const DWORD flags[]={0,1,2,4,8,12,0x20,0x40,0x80,0xa0,0x82,0x21,0x45,0x4c,0x88,0x8c};
                const bool fullscreen=trial<96 ? bool(trial&16):bool((trial-96)&2),suppress=trial<96 && (trial&32);
                DesktopImagePlacement placement; placement.scaleX=1; placement.scaleY=1; placement.offsetX=0; placement.offsetY=0;
                sprites.begin();
                if(trial<96) {
                auto clip=quad(90,70,400,320,0xffffffff); sprites.submit(clip,textures[0],4,fullscreen,placement);
                auto first=quad(20,30,540,400,0xc080b0ff,1+trial%4); first.u0=-0.4f; first.v0=-0.2f; first.u1=2.3f; first.v1=1.9f;
                first.corners[3].color=0x4040ff20; first.corners[2].x+=30;
                if(trial>=64) { first.shader=shader; first.constants=&constants; }
                sprites.submit(first,textures[trial&1],flags[trial%16],fullscreen,placement);
                auto second=quad(250,170,230,250,0x9090ff50,2); sprites.submit(second,textures[1],flags[(trial+7)%16],fullscreen,placement);
                sprites.submit(quad(560,120,150,210,0xb0ff8040),nullptr,2,fullscreen,placement); sprites.finish();
                require(sprites.batches().size()==4,"sprite batch transitions differ");
                } else {
                    const auto fontIndex=(trial-96)/4; const auto variant=(trial-96)%4;
                    auto* fontTexture=fontTextures[fontIndex];
                    for(unsigned line=0;line<menuLabels.size();++line) {
                        TextLinePlacement textPlacement{400.f,60.f+line*80.f,520.f,10.f,0.5f,variant==3 ? 2U:variant};
                        const auto layout=layoutTextLine(*fonts[fontIndex],menuLabels[line].view(),fontTexture->allocatedWidth(),fontTexture->allocatedHeight(),
                            fontTexture->scaleU(),fontTexture->scaleV(),textPlacement);
                        require(!layout.letters.empty(),"Localized menu text produced no letters");
                        submitTextLetters(sprites,layout,fontTexture,0xffffdd80,0.875f,1,0x20,fullscreen,placement);
                    }
                    sprites.finish(); require(sprites.batches().size()==1,"Font letters did not share the atlas batch");
                }
                auto reset=[&] {
                    render.setup(settings,presentation); device->SetVertexShader(0x142); device->SetRenderState(D3DRS_STENCILENABLE,0);
                    device->SetRenderState(D3DRS_STENCILREF,0); device->SetRenderState(D3DRS_STENCILFUNC,8);
                    device->SetRenderState(D3DRS_STENCILMASK,0xffffffffU); device->SetRenderState(D3DRS_STENCILWRITEMASK,0xffffffffU);
                    device->SetRenderState(D3DRS_STENCILZFAIL,1); device->SetRenderState(D3DRS_STENCILPASS,1);
                    device->SetTextureStageState(0,D3DTSS_MAGFILTER,1); device->SetTextureStageState(0,D3DTSS_MINFILTER,1);
                    require(SUCCEEDED(device->Clear(0,nullptr,7,0xff17293b,0,0)),"scene clear failed");
                    require(SUCCEEDED(device->BeginScene()),"scene begin failed");
                };
                SpriteDeviceTrace actualTrace(device),expectedTrace(device);
                reset(); const DWORD actualCount=drawSpriteBatches(actualTrace.device(),buffers,sprites.batches(),render.transposedViewProjection(),fullscreen,suppress);
                require(SUCCEEDED(device->EndScene()),"recovered end scene failed"); const auto actual=pixels(device);
                std::vector<Node> nodes(sprites.batches().size()+1); Node* head=&nodes[0]; head->next=&nodes[1]; head->previous=&nodes.back();
                for(unsigned i=0;i<sprites.batches().size();++i) {
                    auto& node=nodes[i+1]; node.next=i+2<nodes.size() ? &nodes[i+2]:head; node.previous=&nodes[i];
                    std::memcpy(node.words,&sprites.batches()[i],32); node.words[8]=0;
                    if(sprites.batches()[i].constants) node.words[7]=reinterpret_cast<DWORD>(&originalConstants);
                }
                std::array<unsigned char,680> object{}; put(object,0x10,expectedTrace.device()); put(object,0x15c,buffers.indices()); put(object,0x160,buffers.vertices());
                put(object,0x180,head); put(object,0x98,render.transposedViewProjection()); put(object,0x110,static_cast<unsigned char>(fullscreen));
                put(object,0x21c,static_cast<unsigned char>(suppress));
                reset(); draw(object.data()); require(SUCCEEDED(device->EndScene()),"original end scene failed"); const auto expected=pixels(device);
                if(actualTrace.calls!=expectedTrace.calls) {
                    for(unsigned i=0;i<actualTrace.calls.size() && i<expectedTrace.calls.size();++i) if(actualTrace.calls[i]!=expectedTrace.calls[i]) {
                        std::cerr << "trial " << trial << " API call " << i << " actual"; for(auto value:actualTrace.calls[i]) std::cerr << ' ' << value;
                        std::cerr << " original"; for(auto value:expectedTrace.calls[i]) std::cerr << ' ' << value; std::cerr << '\n'; break;
                    }
                    throw std::runtime_error("original sprite API calls differ");
                }
                DWORD expectedCount; std::memcpy(&expectedCount,object.data()+0x1e8,4); require(actualCount==expectedCount,"triangle metric differs");
                if(actual!=expected) {
                    for(unsigned i=0;i<actual.size();++i) if(actual[i]!=expected[i]) { std::cerr << "trial " << trial << " byte " << i << " actual " << unsigned(actual[i]) << " original " << unsigned(expected[i]) << '\n'; break; }
                    throw std::runtime_error("original GPU pixels differ");
                }
                for(unsigned i=0;i<actual.size();i+=4) if(actual[i]!=0x3b || actual[i+1]!=0x29 || actual[i+2]!=0x17) ++drawnPixels;
                comparedPixels+=actual.size()/4; ++scenarios;
            }
            require(drawnPixels>10000,"sprite fixture rendered no visible pixels");
            shader->preResetGraphics(); shader->releaseReference();
            for(auto* font:fonts) font->releaseReference(); for(auto* texture:fontTextures) texture->releaseReference(); fontFiles()=nullptr;
            for(auto* texture:textures) texture->releaseReference(); manager.clearUseless(); textureEnvironment()={};
            shaderEnvironment()={};
        }
        device->Release(); device=nullptr; d3d->Release(); d3d=nullptr; DestroyWindow(window); window=nullptr;
        FreeLibrary(runtime); runtime=nullptr; FreeLibrary(reference); reference=nullptr;
        std::cout << "{\"scenarios\":" << scenarios << ",\"font_scenarios\":12,\"pixels_compared\":" << comparedPixels << ",\"nonbackground_pixels\":" << drawnPixels << "}\n";
    } catch(const std::exception& error) {
        std::cerr << error.what() << '\n'; if(device) device->Release(); if(d3d) d3d->Release(); if(window) DestroyWindow(window); if(runtime) FreeLibrary(runtime); if(reference) FreeLibrary(reference); return 1;
    } catch(const char* error) { std::cerr << error << '\n'; return 1; }
}
