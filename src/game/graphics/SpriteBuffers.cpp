#include "SpriteBuffers.h"
namespace farmar::recovered {
void SpriteBuffers::release() noexcept {
    if(indices_) indices_->Release();
    if(vertices_) vertices_->Release();
    if(triangles_) triangles_->Release();
    indices_=nullptr; vertices_=nullptr; triangles_=nullptr;
}
SpriteBuffers::~SpriteBuffers() { release(); }
void SpriteBuffers::resize(unsigned sprites) {
    release();
    indexCount_=sprites*6; vertexCount_=sprites*4;
    const DWORD usage=D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY; // original 0x208
    if(device_->CreateIndexBuffer(indexCount_*2,usage,D3DFMT_INDEX16,D3DPOOL_DEFAULT,&indices_)!=0)
        device_->CreateIndexBuffer(indexCount_*2,usage,D3DFMT_INDEX16,D3DPOOL_MANAGED,&indices_);
    if(device_->CreateVertexBuffer(vertexCount_*sizeof(SpriteVertex),usage,0x142,D3DPOOL_DEFAULT,&vertices_)!=0)
        device_->CreateVertexBuffer(vertexCount_*sizeof(SpriteVertex),usage,0x142,D3DPOOL_MANAGED,&vertices_);
    sprites_=sprites;
}
}
