#pragma once
#include "Direct3D8.h"

namespace farmar::recovered {
struct TextureImageSize { unsigned long width, height; };
// Functional portion of 0050f580; cGraphics singleton/resource ABI is separate.
void getJngTexture(IDirect3DDevice8* device,const void* source,const unsigned long& sourceSize,
                   IDirect3DTexture8*& texture,TextureImageSize& imageSize);
}
