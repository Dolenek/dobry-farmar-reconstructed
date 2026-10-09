#include "JngTexture.h"
#include "JNGLoad.h"
#include <cstring>

namespace farmar::recovered {
void getJngTexture(IDirect3DDevice8* device,const void* source,const unsigned long&,
                   IDirect3DTexture8*& texture,TextureImageSize& imageSize) {
    unsigned long* pixels=nullptr;
    readMNG(source,imageSize.width,imageSize.height,pixels);
    unsigned width=1,height=1;
    while (width<imageSize.width) width*=2;
    while (height<imageSize.height) height*=2;
    // Original: one mip, usage 0, format 0x15, pool 1; HRESULTs are ignored.
    device->CreateTexture(width,height,1,0,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED,&texture);
    D3DLOCKED_RECT locked;
    texture->LockRect(0,&locked,nullptr,0);
    auto* row=static_cast<unsigned long*>(locked.pBits);
    for (unsigned y=0;y<imageSize.height;++y) {
        std::memcpy(row,pixels+imageSize.width*y,imageSize.width*4);
        row+=locked.Pitch>>2;
    }
    texture->UnlockRect(0);
    freeData(pixels);
}
}
