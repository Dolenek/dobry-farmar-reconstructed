#pragma once
#include "Direct3D8.h"
#include <vector>
#include <cstddef>
namespace farmar::recovered {
struct NativeImagePlane {
    D3DFORMAT format;
    unsigned width,height,pitch;
    std::vector<unsigned char> bytes;
};
struct NativeImage {
    std::vector<NativeImagePlane> planes; // DDS faces, then mip levels
    unsigned faces=1,mipLevels=1;
};
// Original image-reader branches used by shipped JPEG/DDS/TGA/BMP data. These decode
// storage, not D3DX texture requirements, format fallback or surface filtering.
// Other image formats / BMP compression / TGA indexed formats remain explicit
// unsupported inputs instead of a successful placeholder image.
NativeImage decodeNativeImage(const void* data,std::size_t size);
}
