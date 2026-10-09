#pragma once
#include "Font.h"
#include <string_view>
#include <vector>
namespace farmar::recovered {
struct TextLinePlacement {
    float x=0,y=0,depth=0,spaceWidth=0,multiplier=1;
    unsigned alignment=0; // 0=center, 2=right, other values do not offset
};
struct TextLetter {
    FontGlyph glyph;
    float x,y,width,height,depth;
    unsigned characterIndex;
};
struct TextLineLayout {
    std::vector<TextLetter> letters;
    float width=0,height=0;
};
// Geometry arithmetic from cTextLine::makeLetters, 004dc800. The caller supplies
// already localized/rotated text and handles control visibility/lifetime.
TextLineLayout layoutTextLine(const cFont& font,std::wstring_view text,
    unsigned allocatedWidth,unsigned allocatedHeight,float scaleU,float scaleV,
    const TextLinePlacement& placement,float previousHeight=0);
}
