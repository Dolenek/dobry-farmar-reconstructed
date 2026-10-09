#include "SpriteBatches.h"
#include "Texture.h"
#include "Shader.h"
namespace farmar::recovered {
void SpriteBatches::begin() {
    buffers_.indices()->Lock(0,buffers_.indexCount()*2,reinterpret_cast<BYTE**>(&indices_),D3DLOCK_DISCARD);
    buffers_.vertices()->Lock(0,buffers_.vertexCount()*24,reinterpret_cast<BYTE**>(&vertices_),D3DLOCK_DISCARD);
    current_={}; batches_.clear(); vertexCount_=0; indexCount_=0; active_=true;
}
void SpriteBatches::submit(const SpriteDefinition& sprite,cTexture* texture,DWORD flags,bool fullscreen,
    const DesktopImagePlacement& placement) {
    if(!active_) return;
    auto* handle=texture ? texture->texture():nullptr;
    const DWORD shader=sprite.shader ? sprite.shader->shader():0;
    if(vertexCount_ && (current_.texture!=handle || current_.flags!=flags || current_.shader!=shader || current_.constants!=sprite.constants)) {
        batches_.push_back(current_);
        current_.baseVertex+=current_.vertices;
        current_.startIndex=indexCount_; current_.triangles=0; current_.vertices=0;
    }
    current_.texture=handle; current_.flags=flags; current_.shader=shader; current_.constants=sprite.constants;
    writeSpriteGeometry(sprite,{texture ? texture->scaleU():1.f,texture ? texture->scaleV():1.f},fullscreen,
        placement,vertices_+vertexCount_,indices_+indexCount_,vertexCount_);
    const DWORD vertices=(sprite.subdivisions+1)*(sprite.subdivisions+1),triangles=sprite.subdivisions*sprite.subdivisions*2;
    current_.vertices+=vertices; current_.triangles+=triangles; vertexCount_+=vertices; indexCount_+=triangles*3;
}
void SpriteBatches::finish() {
    if(!active_) return;
    buffers_.indices()->Unlock(); buffers_.vertices()->Unlock();
    if(current_.vertices) batches_.push_back(current_);
    active_=false;
}
DWORD drawSpriteBatches(IDirect3DDevice8* device,const SpriteBuffers& buffers,
    const std::vector<SpriteDrawBatch>& batches,const Matrix4& projection,bool fullscreen,bool suppressTextures) {
    device->SetIndices(buffers.indices(),0);
    device->SetStreamSource(0,buffers.vertices(),24);
    DWORD triangles=0,state=0,stage=0;
    const auto render=[&](D3DRENDERSTATETYPE key,DWORD value) {
        device->GetRenderState(key,&state); if(state!=value) device->SetRenderState(key,value);
    };
    const auto filter=[&](D3DTEXTURESTAGESTATETYPE key,DWORD value) {
        device->GetTextureStageState(0,key,&stage); if(stage!=value) device->SetTextureStageState(0,key,value);
    };
    for(const auto& batch:batches) {
        device->SetTexture(0,suppressTextures ? nullptr:batch.texture);
        if(!(batch.flags&0xc)) render(D3DRS_STENCILENABLE,0);
        else {
            if(batch.flags&4) {
                render(D3DRS_STENCILENABLE,1); render(D3DRS_STENCILFUNC,8); render(D3DRS_STENCILREF,1);
                render(D3DRS_STENCILMASK,0xffffffffU); render(D3DRS_STENCILWRITEMASK,0xffffffffU);
                render(D3DRS_STENCILZFAIL,1); render(D3DRS_STENCILFAIL,1); render(D3DRS_STENCILPASS,3);
                render(D3DRS_ALPHAREF,0x88); render(D3DRS_SRCBLEND,1); render(D3DRS_DESTBLEND,2);
                device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,batch.baseVertex,batch.vertices,batch.startIndex,batch.triangles);
                triangles+=batch.triangles;
                render(D3DRS_ALPHAREF,0); render(D3DRS_STENCILENABLE,0);
            }
            if(batch.flags&8) {
                render(D3DRS_STENCILENABLE,1); render(D3DRS_STENCILREF,1); render(D3DRS_STENCILFUNC,3); render(D3DRS_STENCILPASS,1);
            }
        }
        if(batch.flags&0x41) { render(D3DRS_SRCBLEND,9); render(D3DRS_DESTBLEND,5); }
        else { render(D3DRS_SRCBLEND,5); render(D3DRS_DESTBLEND,6); }
        render(D3DRS_ALPHABLENDENABLE,(batch.flags&2) ? 0:1);
        render(D3DRS_ALPHATESTENABLE,(batch.flags&2) ? 0:1);
        const DWORD mode=(batch.flags&0x20) ? 2:((batch.flags&0x80) ? 3:1);
        filter(D3DTSS_MAGFILTER,mode); filter(D3DTSS_MINFILTER,mode);
        if(!batch.shader) device->SetVertexShader(0x142);
        else {
            device->SetVertexShader(batch.shader);
            device->SetVertexShaderConstant(0,projection.values,4);
            if(batch.constants) for(const auto& value:batch.constants->values)
                device->SetVertexShaderConstant(value.index,value.data,value.vectorCount);
        }
        if(fullscreen) filter(D3DTSS_MAGFILTER,(batch.flags&1) ? 1:3);
        device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,batch.baseVertex,batch.vertices,batch.startIndex,batch.triangles);
        triangles+=batch.triangles;
    }
    return triangles;
}
}
