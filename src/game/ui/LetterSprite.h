#pragma once
#include "TextLineLayout.h"
#include "SpriteBatches.h"
namespace farmar::recovered {
class cTexture;
// cLetter::makeTransform, 0050c500. Changes geometry/UV/color only, preserving
// subdivisions, shader and constants already owned by the sprite.
void updateLetterSprite(SpriteDefinition& sprite,const TextLetter& letter,
    std::uint32_t packedColor,float rootAlpha,float alpha) noexcept;
// Integration of recovered font layout and letter geometry into GPU batches.
// The UI dispatcher remains responsible for visibility and global depth order.
void submitTextLetters(SpriteBatches& batches,const TextLineLayout& line,cTexture* texture,
    std::uint32_t packedColor,float rootAlpha,float alpha,DWORD flags,bool fullscreen,
    const DesktopImagePlacement& placement);
}
