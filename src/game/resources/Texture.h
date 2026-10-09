#pragma once
#include "Resource.h"
#include "JngTexture.h"
#include <functional>

namespace farmar::recovered {
class FileIndex;
// Replaces the original file-manager and graphics singleton call sites. The
// complete Graphics dispatcher must supply non-JNG decoding when reconstructed.
struct TextureEnvironment {
    FileIndex* files=nullptr;
    std::function<void(const void*,const unsigned long&,IDirect3DTexture8*&,TextureImageSize&)> load;
};
TextureEnvironment& textureEnvironment() noexcept;
#pragma pack(push,4)
class cTexture final : public cResource {
public:
    explicit cTexture(const String& name); // 00559150
    ~cTexture() override; // 005585c0
    std::uint32_t resourceType() const override { return 0; } // 005585b0
    void preResetGraphics() override; // 005589b0
    void restoreGraphics() override; // 00558d80
    IDirect3DTexture8* texture() const noexcept { return texture_; }
    float scaleU() const noexcept { return scaleU_; }
    float scaleV() const noexcept { return scaleV_; }
    float halfTexelU() const noexcept { return halfU_; }
    float halfTexelV() const noexcept { return halfV_; }
    std::uint32_t allocatedWidth() const noexcept { return allocatedWidth_; }
    std::uint32_t allocatedHeight() const noexcept { return allocatedHeight_; }
    std::uint32_t imageWidth() const noexcept { return imageWidth_; }
    std::uint32_t imageHeight() const noexcept { return imageHeight_; }
private:
    IDirect3DTexture8* texture_=nullptr; // +24
    float scaleU_=1,scaleV_=1,halfU_=0,halfV_=0;
    std::uint32_t allocatedWidth_=0,allocatedHeight_=0,imageWidth_=0,imageHeight_=0;
    void load(const String& name); // 005580c0
};
#pragma pack(pop)
static_assert(sizeof(cTexture)==72,"Original texture resource layout");
}
