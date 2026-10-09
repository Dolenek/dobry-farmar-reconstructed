#pragma once
#include "Direct3D8.h"
namespace farmar::recovered {
struct SpriteVertex { float x,y,z; DWORD color; float u,v; };
static_assert(sizeof(SpriteVertex)==24,"Original sprite vertex stride");
// Buffer allocation operation 005105e0. Index/vertex population and drawing are
// separate original functions, not implemented by this allocation operation.
class SpriteBuffers {
public:
    explicit SpriteBuffers(IDirect3DDevice8* device) : device_(device) {}
    ~SpriteBuffers();
    SpriteBuffers(const SpriteBuffers&)=delete;
    SpriteBuffers& operator=(const SpriteBuffers&)=delete;
    void resize(unsigned sprites);
    IDirect3DVertexBuffer8* vertices() const noexcept { return vertices_; }
    IDirect3DIndexBuffer8* indices() const noexcept { return indices_; }
    unsigned spriteCount() const noexcept { return sprites_; }
    unsigned vertexCount() const noexcept { return vertexCount_; }
    unsigned indexCount() const noexcept { return indexCount_; }
private:
    IDirect3DDevice8* device_;
    IDirect3DIndexBuffer8* indices_=nullptr;
    IDirect3DVertexBuffer8* vertices_=nullptr;
    IDirect3DVertexBuffer8* triangles_=nullptr; // original +218, released on resize
    unsigned sprites_=0,vertexCount_=0,indexCount_=0;
    void release() noexcept;
};
}
