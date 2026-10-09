#pragma once
#include "SpriteGeometry.h"
#include "GameMatrix.h"
#include <vector>
namespace farmar::recovered {
class cTexture;
struct ShaderConstant { DWORD index; const float* data; DWORD vectorCount; };
// Original draw code iterates a linked sequence in insertion order. Modern
// storage preserves that order, not the original container ABI or ownership.
class ShaderConstants { public: std::vector<ShaderConstant> values; };
struct SpriteDrawBatch {
    IDirect3DTexture8* texture=nullptr;
    DWORD startIndex=0,triangles=0,vertices=0,flags=0,baseVertex=0,shader=0;
    ShaderConstants* constants=nullptr;
};
static_assert(sizeof(SpriteDrawBatch)==32,"Sprite portion of original nine-word batch");
// Restored sprite-only buffer writing/batch transitions and draw path. Full
// cGraphics frame timing, device reset, mesh commands and font overlay remain
// separate work; this class exposes no pretend implementation of those paths.
class SpriteBatches {
public:
    explicit SpriteBatches(SpriteBuffers& buffers) : buffers_(buffers) {}
    void begin(); // buffer/list part of 0051d2d0
    void submit(const SpriteDefinition& sprite,cTexture* texture,DWORD flags,bool fullscreen,
                const DesktopImagePlacement& placement); // 00517f40 + 00517ae0
    void finish(); // unlock/final sprite batch part of 00517010
    const std::vector<SpriteDrawBatch>& batches() const noexcept { return batches_; }
    DWORD vertexCount() const noexcept { return vertexCount_; }
    DWORD indexCount() const noexcept { return indexCount_; }
private:
    SpriteBuffers& buffers_;
    SpriteVertex* vertices_=nullptr;
    std::uint16_t* indices_=nullptr;
    bool active_=false;
    DWORD vertexCount_=0,indexCount_=0;
    SpriteDrawBatch current_;
    std::vector<SpriteDrawBatch> batches_;
};
// Sprite branches of 00515830. Returns the original triangle metric delta,
// counting both stencil-write and color passes. Mesh branches are not exposed.
DWORD drawSpriteBatches(IDirect3DDevice8* device,const SpriteBuffers& buffers,
    const std::vector<SpriteDrawBatch>& batches,const Matrix4& transposedViewProjection,
    bool fullscreen,bool suppressTextures=false);
}
