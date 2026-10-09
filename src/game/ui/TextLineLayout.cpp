#include "TextLineLayout.h"
#include "OriginalArithmetic.h"
#include <cmath>
namespace farmar::recovered {
TextLineLayout layoutTextLine(const cFont& font,std::wstring_view text,
    unsigned allocatedWidth,unsigned allocatedHeight,float scaleU,float scaleV,
    const TextLinePlacement& p,float previousHeight) {
    TextLineLayout result; result.height=previousHeight;
    const float imageWidth=static_cast<float>(originalMultiply(static_cast<float>(static_cast<int>(allocatedWidth)),scaleU));
    const float imageHeight=static_cast<float>(originalMultiply(static_cast<int>(allocatedHeight),scaleV));
    float cursor=0;
    for(unsigned index=0;index<text.size();++index) {
        const auto character=static_cast<std::uint16_t>(text[index]);
        if(character==0x20 || character==0xa0) {
            cursor+=p.spaceWidth;
            result.width=static_cast<float>(originalAdd(originalMultiply(p.multiplier,p.spaceWidth),result.width));
            continue;
        }
        FontGlyph glyph{}; if(!font.getGlyph(character,glyph)) continue;
        const float v0=static_cast<float>(originalSubtract(1.0,glyph.v1)),v1=static_cast<float>(originalSubtract(1.0,glyph.v0));
        const float advance=static_cast<float>(originalMultiply(imageWidth,glyph.advance));
        TextLetter letter{}; letter.glyph={glyph.u0,v1,glyph.u1,v0,glyph.advance}; letter.characterIndex=index;
        const float rawHeight=static_cast<float>(originalMultiply(imageHeight,originalSubtract(v0,v1)));
        letter.width=static_cast<float>(originalMultiply(originalMultiply(originalSubtract(glyph.u1,glyph.u0),imageWidth),p.multiplier));
        letter.height=static_cast<float>(originalMultiply(rawHeight,p.multiplier));
        const float half=static_cast<float>(originalMultiply(advance,0.5));
        cursor=static_cast<float>(originalAdd(originalMultiply(advance,0.5),cursor));
        const float vertical=static_cast<float>(originalMultiply(p.multiplier,0.0));
        letter.x=static_cast<float>(originalAdd(originalMultiply(p.multiplier,cursor),p.x));
        letter.y=vertical+p.y; cursor+=half;
        letter.depth=static_cast<float>(originalAdd(originalMultiply(originalUnsigned(index),0.01f),p.depth));
        result.width=static_cast<float>(originalAdd(originalMultiply(advance,p.multiplier),result.width));
        const float absolute=std::fabs(letter.height); if(result.height<=absolute) result.height=absolute;
        result.letters.push_back(letter);
    }
    float offset=0;
    if(p.alignment==0) offset=static_cast<float>(originalMultiply(result.width,-0.5));
    else if(p.alignment==2) offset=-result.width;
    for(auto& letter:result.letters) { letter.x+=offset; letter.y+=0.f; }
    return result;
}
}
