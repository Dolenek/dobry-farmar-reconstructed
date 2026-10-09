#include "ImageTexture.h"
#include "NativeImage.h"
#include <algorithm>
#include <cstring>
#include <stdexcept>
namespace farmar::recovered {
namespace {
unsigned powerOfTwo(unsigned value) { unsigned result=1; while(result<value) result*=2; return result; }
}
void getImageTexture(IDirect3DDevice8* device,const void* source,const unsigned long& size,
    IDirect3DTexture8*& texture,TextureImageSize& dimensions) {
    DWORD magic=0; if(size>=4) std::memcpy(&magic,source,4);
    if(magic==0x474e4a8bU) { getJngTexture(device,source,size,texture,dimensions); return; }
    const auto image=decodeNativeImage(source,size); const auto& plane=image.planes.front();
    dimensions.width=plane.width; dimensions.height=plane.height;
    D3DCAPS8 caps{}; if(FAILED(device->GetDeviceCaps(&caps))) throw std::runtime_error("Native texture caps unavailable");
    unsigned width=(std::min)(plane.width,static_cast<unsigned>(caps.MaxTextureWidth)),height=(std::min)(plane.height,static_cast<unsigned>(caps.MaxTextureHeight));
    const unsigned aspect=(caps.TextureCaps&D3DPTEXTURECAPS_SQUAREONLY) ? 1:caps.MaxTextureAspectRatio;
    if(aspect) { width=(std::min)(width,aspect*height); height=(std::min)(height,aspect*width); }
    if(caps.TextureCaps&D3DPTEXTURECAPS_POW2) { width=powerOfTwo(width); height=powerOfTwo(height); }
    if(static_cast<unsigned>(plane.format)>0x100) throw std::invalid_argument("Compressed DDS texture upload awaits original filtering verification");
    // Original D3DX considers all supported formats by a recovered scoring
    // algorithm. That fallback is pending; never silently choose a substitute.
    IDirect3D8* d3d=nullptr; D3DDEVICE_CREATION_PARAMETERS creation{}; D3DDISPLAYMODE mode{};
    device->GetDirect3D(&d3d); device->GetCreationParameters(&creation); d3d->GetAdapterDisplayMode(creation.AdapterOrdinal,&mode);
    const HRESULT supported=d3d->CheckDeviceFormat(creation.AdapterOrdinal,creation.DeviceType,mode.Format,0,D3DRTYPE_TEXTURE,plane.format);
    d3d->Release(); if(FAILED(supported)) throw std::runtime_error("Original D3DX texture format fallback remains unrecovered");
    IDirect3DTexture8* result=nullptr;
    if(FAILED(device->CreateTexture(width,height,1,0,plane.format,D3DPOOL_MANAGED,&result))) throw std::runtime_error("Native image CreateTexture failed");
    D3DLOCKED_RECT locked{};
    if(FAILED(result->LockRect(0,&locked,nullptr,0))) { result->Release(); throw std::runtime_error("Native image texture lock failed"); }
    const unsigned pixelBytes=plane.format==D3DFMT_L8 ? 1:(plane.format==D3DFMT_R8G8B8 ? 3:(plane.format<=D3DFMT_X8R8G8B8 ? 4:2));
    const unsigned rows=(std::min)(height,plane.height),columns=(std::min)(width,plane.width);
    // D3DX_FILTER_NONE copies overlapping pixels and writes zero Float4 outside
    // the source extent (00595901). Pitch padding is not part of the image.
    for(unsigned y=0;y<height;++y) {
        auto* output=static_cast<unsigned char*>(locked.pBits)+std::size_t(y)*locked.Pitch;
        std::memset(output,0,width*pixelBytes);
        if(y<rows) std::memcpy(output,plane.bytes.data()+std::size_t(y)*plane.pitch,columns*pixelBytes);
        if(plane.format==D3DFMT_X8R8G8B8 && (width!=plane.width || height!=plane.height))
            for(unsigned x=0;x<width;++x) output[x*4+3]=0xff;
    }
    result->UnlockRect(0); texture=result;
}
}
