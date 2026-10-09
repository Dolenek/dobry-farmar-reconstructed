#include "LetterSprite.h"
#include "GameColor.h"
namespace farmar::recovered {
void updateLetterSprite(SpriteDefinition& sprite,const TextLetter& letter,
    std::uint32_t packedColor,float rootAlpha,float alpha) noexcept {
    sprite.u0=letter.glyph.u0; sprite.v0=letter.glyph.v0;
    sprite.u1=letter.glyph.u1; sprite.v1=letter.glyph.v1;
    Color color=Color::fromARGB(packedColor);
    // The original replaces the packed color's alpha with these two factors.
    color.a=static_cast<float>(static_cast<double>(rootAlpha)*alpha);
    const auto packed=color.toARGB();
    const float left=static_cast<float>(static_cast<float>(-static_cast<double>(letter.width)*0.5)+letter.x);
    const float top=static_cast<float>(static_cast<float>(-static_cast<double>(letter.height)*0.5)+letter.y);
    const float right=static_cast<float>(static_cast<float>(static_cast<double>(letter.width)*0.5)+letter.x);
    const float bottom=static_cast<float>(static_cast<double>(letter.height)*0.5+letter.y);
    sprite.corners[0]={left,top,packed}; sprite.corners[1]={right,top,packed};
    sprite.corners[2]={left,static_cast<float>(static_cast<float>(static_cast<double>(letter.height)*0.5)+letter.y),packed};
    sprite.corners[3]={right,bottom,packed};
}
void submitTextLetters(SpriteBatches& batches,const TextLineLayout& line,cTexture* texture,
    std::uint32_t packedColor,float rootAlpha,float alpha,DWORD flags,bool fullscreen,
    const DesktopImagePlacement& placement) {
    for(const auto& letter:line.letters) {
        SpriteDefinition sprite{}; sprite.subdivisions=1;
        updateLetterSprite(sprite,letter,packedColor,rootAlpha,alpha);
        batches.submit(sprite,texture,flags,fullscreen,placement);
    }
}
}
