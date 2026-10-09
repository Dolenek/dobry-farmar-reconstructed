#pragma once
#include "SpriteBuffers.h"
#include "GraphicsPresentation.h"
#include <cstddef>
#include <cstdint>
namespace farmar::recovered {
class cShader;
class ShaderConstants;
struct SpriteCorner { float x,y; std::uint32_t color; };
// Geometry storage of original iGraphics::sSprite. Constructors/copy/shader
// ownership are separate recovery work; callers supply every field explicitly.
struct SpriteDefinition {
    SpriteCorner corners[4]; // TL, TR, BL, BR; +0,+c,+18,+24
    std::uint32_t subdivisions; // +30
    cShader* shader; // +34, shader resource (type 3), not a texture
    float u0,v0,u1,v1; // +38,+3c,+40,+44
    ShaderConstants* constants; // +48, ordered vertex shader constants
};
static_assert(sizeof(SpriteDefinition)==76 && offsetof(SpriteDefinition,shader)==52,"Original sprite geometry storage");
struct TextureScale { float u,v; };
// Vertex/index-writing portion of 00517f40. Batch boundaries and render commands
// are recovered separately; this operation does not approximate their behavior.
void writeSpriteGeometry(const SpriteDefinition& sprite,TextureScale textureScale,bool fullscreen,
    const DesktopImagePlacement& placement,SpriteVertex* vertices,std::uint16_t* indices,std::uint32_t baseVertex=0) noexcept;
}
