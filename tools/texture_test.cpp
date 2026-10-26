// GPU test fixture, not the original cGraphics initialization or game renderer.
#include "JngTexture.h"
#include "ImageTexture.h"
#include "NativeImage.h"
#include "FileIndex.h"
#include "Texture.h"
#include "ResourceManager.h"
#include "XmlResource.h"
#include "Settings.h"
#include <cstring>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
using namespace farmar::recovered;
namespace {
String ansi(const wchar_t* text) {
    int count=WideCharToMultiByte(CP_ACP,0,text,-1,nullptr,0,nullptr,nullptr);
    std::string output(count,'\0');
    WideCharToMultiByte(CP_ACP,0,text,-1,output.data(),count,nullptr,nullptr);
    return String(output.c_str());
}
unsigned power(unsigned value) { unsigned out=1; while(out<value) out*=2; return out; }
void require(bool value,const char* message) { if(!value) throw std::runtime_error(message); }
}
int wmain(int argc,wchar_t** argv) {
    if(argc!=3) return 2;
    HMODULE runtime=nullptr,reference=nullptr;
    IDirect3D8* d3d=nullptr;
    IDirect3DDevice8* device=nullptr;
    HWND window=nullptr;
    try {
        wchar_t system[MAX_PATH];
        require(GetSystemDirectoryW(system,MAX_PATH)!=0,"System directory unavailable");
        const auto dll=std::wstring(system)+L"\\d3d8.dll";
        runtime=LoadLibraryW(dll.c_str()); require(runtime!=nullptr,"System Direct3D8 runtime unavailable");
        using Factory=IDirect3D8*(WINAPI*)(UINT);
        auto factory=reinterpret_cast<Factory>(GetProcAddress(runtime,"Direct3DCreate8"));
        require(factory!=nullptr,"Direct3DCreate8 unavailable");
        d3d=factory(120); require(d3d!=nullptr,"Original SDK version 120 unavailable");
        D3DDISPLAYMODE mode{};
        require(SUCCEEDED(d3d->GetAdapterDisplayMode(0,&mode)),"Display mode query failed");
        D3DADAPTER_IDENTIFIER8 adapter{}; d3d->GetAdapterIdentifier(0,0,&adapter);
        // Hidden test-owned window and device, independent of game boot parameters.
        window=CreateWindowExW(0,L"STATIC",L"Texture comparison",WS_POPUP,0,0,64,64,
            nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);
        require(window!=nullptr,"Texture test window failed");
        D3DPRESENT_PARAMETERS parameters{};
        parameters.BackBufferWidth=64; parameters.BackBufferHeight=64;
        parameters.BackBufferFormat=mode.Format; parameters.BackBufferCount=1;
        parameters.SwapEffect=D3DSWAPEFFECT_DISCARD;
        parameters.hDeviceWindow=window; parameters.Windowed=TRUE;
        require(SUCCEEDED(d3d->CreateDevice(0,D3DDEVTYPE_HAL,window,D3DCREATE_SOFTWARE_VERTEXPROCESSING,
            &parameters,&device)),"HAL test device creation failed");
        reference=LoadLibraryW(argv[2]); require(reference!=nullptr,"Original JNG oracle DLL unavailable");
        using Read=void(__cdecl*)(const void*,unsigned long&,unsigned long&,unsigned long*&);
        using Free=void(__cdecl*)(unsigned long*&);
        auto decode=reinterpret_cast<Read>(GetProcAddress(reference,"?readMNG@@YAXPBXAAK1AAPAK@Z"));
        auto release=reinterpret_cast<Free>(GetProcAddress(reference,"?freeData@@YAXAAPAK@Z"));
        require(decode && release,"Original JNG exports unavailable");
        FileIndex files(ansi(argv[1]),String("pack"));
        textureEnvironment().files=&files;
        textureEnvironment().load=[&](const void* source,const unsigned long& size,IDirect3DTexture8*& texture,TextureImageSize& image) {
            getImageTexture(device,source,size,texture,image);
        };
        unsigned compared=0,nativeCompared=0,maxWidth=0,maxHeight=0;
        std::uint64_t pixelsCompared=0,nativePixelsCompared=0;
        D3DCAPS8 caps{}; device->GetDeviceCaps(&caps);
        {
        ResourceManager manager(files);
        auto* settingsXML=manager.getXML(String("SYS/SETTINGS.XML"));
        require(settingsXML->resourceType()==1 && manager.cachedCount()==1,"XML resource not cached");
        require(manager.getXML(String("sys/settings.xml"))==settingsXML && settingsXML->referenceCount()==2,"XML cache/ref differs");
        require(settingsXML->releaseReference() && settingsXML->referenceCount()==2,"XML release must return true without decrement");
        bool wrongType=false;
        try { manager.getTexture(String("sys/settings.xml")); } catch(const char* message) { wrongType=std::strcmp(message,"Wrong resource type.")==0; }
        require(wrongType,"Cached wrong type was accepted");
        delete settingsXML;
        require(manager.cachedCount()==0,"XML destructor did not erase cache");
        Settings settings(files,[&](const String& name) { return manager.loadSettingsXML(name); });
        require(manager.cachedCount()==0 && !settings.sections().empty(),"Settings must delete cached XML after reading");
        for(const auto& item:files.files()) {
            if(item.first.size()<4) continue;
            const auto suffix=item.first.substr(item.first.size()-4);
            if(suffix!=".jng" && suffix!=".dds" && suffix!=".tga" && suffix!=".bmp" && suffix!=".jpg") continue;
            auto file=files.open(String(item.first));
            std::vector<unsigned char> bytes(file->size());
            require(file->read(bytes.data(),static_cast<unsigned>(bytes.size()))==bytes.size(),"Short JNG read");
            file.reset();
            auto* owned=manager.getTexture(String(item.first));
            auto& resource=*owned;
            require(manager.getTexture(String(item.first))==owned && resource.referenceCount()==2,"Texture cache not reused");
            require(manager.getResourceName(owned).view()==item.first,"Reverse cache name differs");
            auto* texture=resource.texture();
            require(resource.resourceType()==0 && resource.resourceName().view()==item.first,"Texture resource type/name differs");
            require(!resource.releaseReference(),"Cached reference decrement differs");
            require(resource.releaseReference(),"Resource zero reference does not signal release");
            DWORD magic=0; std::memcpy(&magic,bytes.data(),4); const bool isJng=magic==0x474e4a8bU;
            unsigned long width=0,height=0,*expected=nullptr;
            NativeImage native; D3DFORMAT format=D3DFMT_A8R8G8B8; unsigned pixelBytes=4,sourcePitch=0;
            const unsigned char* sourcePixels=nullptr;
            if(isJng) { decode(bytes.data(),width,height,expected); sourcePixels=reinterpret_cast<const unsigned char*>(expected); sourcePitch=width*4; }
            else {
                native=decodeNativeImage(bytes.data(),bytes.size()); const auto& plane=native.planes.front();
                width=plane.width; height=plane.height; format=plane.format; sourcePitch=plane.pitch; sourcePixels=plane.bytes.data();
                pixelBytes=format==D3DFMT_A4R4G4B4 ? 2:4;
                require(format==D3DFMT_A4R4G4B4 || format==D3DFMT_A8R8G8B8 || format==D3DFMT_X8R8G8B8,"Unexpected shipped native format");
            }
            D3DSURFACE_DESC description{};
            require(SUCCEEDED(texture->GetLevelDesc(0,&description)),"Texture description failed");
            require(resource.imageWidth()==width && resource.imageHeight()==height,"Original image dimensions differ");
            const bool requiresPower=isJng || (caps.TextureCaps&D3DPTEXTURECAPS_POW2);
            require(description.Width==(requiresPower ? power(width):width) && description.Height==(requiresPower ? power(height):height),"Allocated original required dimensions differ");
            require(description.Format==format && description.Pool==D3DPOOL_MANAGED &&
                description.Usage==0 && texture->GetLevelCount()==1,"Original texture format/pool/mips differ");
            D3DLOCKED_RECT rectangle;
            require(SUCCEEDED(texture->LockRect(0,&rectangle,nullptr,D3DLOCK_READONLY)),"Texture read lock failed");
            for(unsigned y=0;y<height;++y)
                require(std::memcmp(static_cast<const unsigned char*>(rectangle.pBits)+rectangle.Pitch*y,
                    sourcePixels+sourcePitch*y,width*pixelBytes)==0,"Reference image differs after GPU texture upload");
            texture->UnlockRect(0);
            require(resource.scaleU()==static_cast<float>(width)/description.Width &&
                resource.scaleV()==static_cast<float>(height)/description.Height &&
                resource.halfTexelU()==0.5f/description.Width && resource.halfTexelV()==0.5f/description.Height,
                "Original UV scale/half-texel values differ");
            manager.preResetGraphics();
            require(resource.texture()==nullptr,"Texture pre-reset did not clear handle");
            manager.restoreGraphics();
            require(resource.texture()!=nullptr,"Texture restore did not reload resource");
            require(SUCCEEDED(resource.texture()->LockRect(0,&rectangle,nullptr,D3DLOCK_READONLY)),"Restored texture read lock failed");
            for(unsigned y=0;y<height;++y)
                require(std::memcmp(static_cast<const unsigned char*>(rectangle.pBits)+rectangle.Pitch*y,
                    sourcePixels+sourcePitch*y,width*pixelBytes)==0,"Restored texture differs from reference image");
            resource.texture()->UnlockRect(0); if(isJng) release(expected);
            maxWidth=(std::max)(maxWidth,description.Width); maxHeight=(std::max)(maxHeight,description.Height);
            if(isJng) { pixelsCompared+=static_cast<std::uint64_t>(width)*height; ++compared; }
            else { nativePixelsCompared+=static_cast<std::uint64_t>(width)*height; ++nativeCompared; }
            manager.clearUseless();
            require(manager.cachedCount()==0,"Unused texture was not deleted/erased");
        }
        }
        std::cout << "{\"textures_compared\":" << compared << ",\"pixels_compared\":" << pixelsCompared
            << ",\"native_textures_compared\":" << nativeCompared << ",\"native_source_pixels_compared\":" << nativePixelsCompared
            << ",\"texture_caps\":" << caps.TextureCaps
            << ",\"maximum_allocated_width\":" << maxWidth << ",\"maximum_allocated_height\":" << maxHeight
            << ",\"resource_reload_checks\":" << compared+nativeCompared
            << ",\"manager_texture_cache_checks\":" << compared+nativeCompared << ",\"manager_xml_settings_checks\":6"
            << ",\"adapter\":" << std::quoted(adapter.Description) << "}\n";
        textureEnvironment()={};
        device->Release(); d3d->Release(); DestroyWindow(window);
        FreeLibrary(reference); FreeLibrary(runtime);
    } catch(const std::exception& error) {
        if(device) device->Release(); if(d3d) d3d->Release(); if(window) DestroyWindow(window);
        if(reference) FreeLibrary(reference); if(runtime) FreeLibrary(runtime);
        std::cerr << error.what() << '\n'; return 1;
    } catch(const char* error) { std::cerr << error << '\n'; return 1; }
}
