#pragma once
#include "GraphicsPresentation.h"
#include "GameMatrix.h"
#include "SpriteBuffers.h"
namespace farmar::recovered {
// Original 00513130: twelve black vertices around the 4:3 desktop image.
class DesktopBorders {
public:
    explicit DesktopBorders(IDirect3DDevice8* device) : device_(device) {}
    ~DesktopBorders();
    DesktopBorders(const DesktopBorders&)=delete;
    DesktopBorders& operator=(const DesktopBorders&)=delete;
    void create(const D3DDISPLAYMODE& desktop,const DesktopImagePlacement& placement);
    IDirect3DVertexBuffer8* vertices() const noexcept { return vertices_; }
private:
    IDirect3DDevice8* device_;
    IDirect3DVertexBuffer8* vertices_=nullptr;
};
// Original 00514920 state/transform setup, separated from the whole cGraphics
// singleton ABI and later batch/draw methods still under reconstruction.
class GraphicsRenderState {
public:
    explicit GraphicsRenderState(IDirect3DDevice8* device);
    void setup(const PrimaryGraphicsSettings& settings,const GraphicsPresentation& presentation);
    const Matrix4& projection() const noexcept { return projection_; }
    const Matrix4& imageTransform() const noexcept { return imageTransform_; }
    const Matrix4& transposedViewProjection() const noexcept { return viewProjection_; }
    IDirect3DVertexBuffer8* borderVertices() const noexcept { return borders_.vertices(); }
private:
    IDirect3DDevice8* device_;
    DesktopBorders borders_;
    D3DMATERIAL8 material_{};
    Matrix4 projection_,view_,imageTransform_,viewProjection_;
};
}
