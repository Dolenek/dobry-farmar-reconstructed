#pragma once
#include "JngTexture.h"
namespace farmar::recovered {
// 00515380 dispatch plus shipped native-image path of 00592e55/00592561 and
// filter NONE (00595901). Other formats/format fallback remain explicit errors.
void getImageTexture(IDirect3DDevice8* device,const void* source,const unsigned long& size,
                     IDirect3DTexture8*& texture,TextureImageSize& dimensions);
}
