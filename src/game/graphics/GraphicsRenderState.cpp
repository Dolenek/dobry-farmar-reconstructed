#include "GraphicsRenderState.h"
#include <cstring>
namespace farmar::recovered {
DesktopBorders::~DesktopBorders() { if(vertices_) vertices_->Release(); }
void DesktopBorders::create(const D3DDISPLAYMODE& desktop,const DesktopImagePlacement& placement) {
    if(vertices_) return;
    if(device_->CreateVertexBuffer(0x120,0x208,0x142,D3DPOOL_DEFAULT,&vertices_)!=0)
        device_->CreateVertexBuffer(0x120,0x208,0x142,D3DPOOL_MANAGED,&vertices_);
    SpriteVertex* vertices=nullptr;
    vertices_->Lock(0,0x120,reinterpret_cast<BYTE**>(&vertices),D3DLOCK_DISCARD);
    std::memset(vertices,0,0x120);
    for(unsigned i=0;i<12;++i) vertices[i].color=0xff000000U;
    if(placement.offsetX<=0) {
        vertices[1].x=vertices[2].x=vertices[4].x=static_cast<float>(desktop.Width);
        vertices[2].y=vertices[4].y=vertices[5].y=placement.offsetY;
        for(unsigned i=0;i<6;++i) {
            vertices[i+6].x=vertices[i].x;
            vertices[i+6].y=static_cast<float>(static_cast<double>(placement.height)+vertices[i].y+placement.offsetY);
        }
    } else {
        vertices[1].x=vertices[2].x=vertices[4].x=placement.offsetX;
        vertices[1].y=placement.offsetY;
        vertices[2].y=vertices[4].y=vertices[5].y=static_cast<float>(desktop.Height);
        for(unsigned i=0;i<6;++i) {
            vertices[i+6].x=static_cast<float>(static_cast<double>(placement.width)+placement.offsetX+vertices[i].x);
            vertices[i+6].y=vertices[i].y;
        }
    }
    vertices_->Unlock();
}
GraphicsRenderState::GraphicsRenderState(IDirect3DDevice8* device) : device_(device),borders_(device) {
    const D3DCOLORVALUE white{1,1,1,1};
    material_.Diffuse=white; material_.Ambient=white; material_.Specular=white; material_.Emissive=white;
    material_.Power=7;
}
void GraphicsRenderState::setup(const PrimaryGraphicsSettings& settings,const GraphicsPresentation& presentation) {
    struct Render { D3DRENDERSTATETYPE type; DWORD value; };
    const Render states[]={{D3DRS_ALPHABLENDENABLE,1},{D3DRS_ALPHATESTENABLE,1},
        {D3DRS_SRCBLEND,5},{D3DRS_DESTBLEND,6},{D3DRS_ALPHAFUNC,5},{D3DRS_BLENDOP,1},
        {D3DRS_ALPHAREF,0},{D3DRS_LIGHTING,0},{D3DRS_CULLMODE,1},{D3DRS_ZENABLE,0},{D3DRS_ZWRITEENABLE,0}};
    DWORD existing=0;
    for(const auto& state:states) {
        device_->GetRenderState(state.type,&existing);
        if(existing!=state.value) device_->SetRenderState(state.type,state.value);
    }
    struct Stage { D3DTEXTURESTAGESTATETYPE type; DWORD value; };
    const Stage stages[]={{D3DTSS_ALPHAOP,4},{D3DTSS_ALPHAARG1,0},{D3DTSS_ALPHAARG2,2},
        {D3DTSS_COLOROP,4},{D3DTSS_COLORARG1,0},{D3DTSS_COLORARG2,2},
        {D3DTSS_TEXCOORDINDEX,0},{D3DTSS_ADDRESSU,3},{D3DTSS_ADDRESSV,3}};
    existing=0;
    for(const auto& state:stages) {
        device_->GetTextureStageState(0,state.type,&existing);
        if(existing!=state.value) device_->SetTextureStageState(0,state.type,state.value);
    }
    if(!settings.fullscreen) {
        orthographicMatrix(projection_,0,static_cast<float>(settings.width),static_cast<float>(settings.height),0,-3000,3000);
        imageTransform_=Matrix4::identity();
    } else {
        const auto& mode=presentation.desktopMode(); const auto& placement=presentation.imagePlacement();
        orthographicMatrix(projection_,0,static_cast<float>(mode.Width),static_cast<float>(mode.Height),0,-3000,3000);
        // Original 004ed010 multiplies the scale matrix by translation here.
        // These sparse operands have exactly this diagonal/translation result.
        imageTransform_=Matrix4::identity();
        imageTransform_.values[0]=placement.scaleX; imageTransform_.values[5]=placement.scaleY;
        imageTransform_.values[12]=placement.offsetX; imageTransform_.values[13]=placement.offsetY;
        borders_.create(mode,placement);
    }
    view_=Matrix4::identity();
    device_->SetMaterial(&material_);
    D3DMATRIX nativeProjection,nativeIdentity;
    std::memcpy(&nativeProjection,&projection_,sizeof(Matrix4)); std::memcpy(&nativeIdentity,&view_,sizeof(Matrix4));
    device_->SetTransform(D3DTS_PROJECTION,&nativeProjection);
    device_->SetTransform(D3DTS_VIEW,&nativeIdentity);
    device_->SetTransform(D3DTS_WORLD,&nativeIdentity);
    multiplyMatrix(viewProjection_,view_,projection_); transposeMatrix(viewProjection_,viewProjection_);
}
}
