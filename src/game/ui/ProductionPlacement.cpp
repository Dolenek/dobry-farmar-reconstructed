#include "ProductionPlacement.h"
#include "OriginalArithmetic.h"
#include <cmath>
#include <cstring>
#include <stdexcept>
namespace farmar::recovered {
namespace {
ProductionRectangle bounds(const Control& control) {
    const auto& g=control.geometry();const float x=float(originalSubtract(g.x,originalMultiply(g.width,0.5))),y=float(originalSubtract(g.y,originalMultiply(g.height,0.5)));
    return {x,y,float(originalAdd(x,g.width)),float(originalAdd(y,g.height))};
}
int integerCoordinate(double value) {
    // _ftol2 returns a signed 64-bit integer; callers use its low EAX word.
    // Nonfinite/out-of-range coordinate input is outside the valid mask scope.
    if(!std::isfinite(value) || value<=-9223372036854775808.0 || value>=9223372036854775808.0) throw std::runtime_error("Production mask coordinate outside original valid range");
    const auto converted=static_cast<std::int64_t>(value);const auto low=static_cast<std::uint32_t>(converted);int result;std::memcpy(&result,&low,4);return result;
}
unsigned maskIndex(unsigned count,float relative,float inverse) {
    const auto scaled=originalMultiply(originalMultiply(originalUnsigned(count-1),relative),inverse);
    return static_cast<unsigned>(integerCoordinate(scaled<0 ? 0:scaled));
}
}
bool productionPointInside(const ProductionRectangle& rectangle,const Point2& point) noexcept {
    return rectangle[0]<=point[0] && point[0]<=rectangle[2] && rectangle[1]<=point[1] && point[1]<=rectangle[3];
}
bool productionRectangleContains(const ProductionRectangle& rectangle,const Control* control) noexcept {
    if(!control) return false;const auto b=bounds(*control);
    return productionPointInside(rectangle,{b[0],b[1]}) && productionPointInside(rectangle,{b[2],b[1]})
        && productionPointInside(rectangle,{b[2],b[3]}) && productionPointInside(rectangle,{b[0],b[3]});
}
bool productionRectangleTouches(const ProductionRectangle& rectangle,const Control* control) noexcept {
    if(!control) return false;const auto b=bounds(*control);
    return productionPointInside(rectangle,{b[0],b[1]}) || productionPointInside(rectangle,{b[2],b[1]})
        || productionPointInside(rectangle,{b[2],b[3]}) || productionPointInside(rectangle,{b[0],b[3]})
        || productionPointInside(b,{rectangle[0],rectangle[1]}) || productionPointInside(b,{rectangle[2],rectangle[1]})
        || productionPointInside(b,{rectangle[2],rectangle[3]}) || productionPointInside(b,{rectangle[0],rectangle[3]});
}
bool productionMasksOverlap(const Control& first,const Control& second,const std::vector<ClickMaskColumn>* firstMask,const std::vector<ClickMaskColumn>* secondMask) {
    if(!firstMask && !secondMask) return true; // original early return, before rectangles
    if(!firstMask || !secondMask || firstMask->empty() || secondMask->empty()) throw std::runtime_error("Original production collision requires both nonempty masks");
    const auto& a=first.geometry();const auto& b=second.geometry();
    float overlapX=float(originalSubtract(originalMultiply(originalAdd(a.width,b.width),0.5),std::fabs(originalSubtract(a.x,b.x))));
    auto overlapY=originalSubtract(originalMultiply(originalAdd(a.height,b.height),0.5),std::fabs(originalSubtract(a.y,b.y)));
    if(overlapX<0 || overlapY<0) return false;
    const float width=a.width<b.width ? a.width:b.width;if(width<=overlapX) overlapX=width;
    const float height=a.height<b.height ? a.height:b.height;if(height<=overlapY) overlapY=height;
    const auto aLeft=originalSubtract(a.x,originalMultiply(a.width,0.5)),aTop=originalSubtract(a.y,originalMultiply(a.height,0.5));
    const float bLeft=float(originalSubtract(b.x,originalMultiply(b.width,0.5))),bTop=float(originalSubtract(b.y,originalMultiply(b.height,0.5)));
    const auto startX=aLeft>bLeft ? aLeft:double(bLeft),startY=aTop>bTop ? aTop:double(bTop);
    const float endX=float(originalAdd(startX,overlapX)),endY=float(originalAdd(startY,overlapY));
    const int firstX=integerCoordinate(startX),firstY=integerCoordinate(startY);
    const auto ab=bounds(first),bb=bounds(second);
    const float inverseAX=float(originalDivide(1,a.width)),inverseAY=float(originalDivide(1,a.height));
    const float inverseBX=float(originalDivide(1,b.width)),inverseBY=float(originalDivide(1,b.height));
    for(int x=firstX;float(x)<endX;++x) {
        const float dxA=float(originalSubtract(float(x),ab[0])),dxB=float(originalSubtract(float(x),bb[0]));
        for(int y=firstY;float(y)<endY;++y) {
            const float dyA=float(originalSubtract(y,ab[1])),dyB=float(originalSubtract(y,bb[1]));
            const auto& rowA=firstMask->at(maskIndex(unsigned(firstMask->size()),dxA,inverseAX));
            const auto& rowB=secondMask->at(maskIndex(unsigned(secondMask->size()),dxB,inverseBX));
            const auto bitA=maskIndex(rowA.bitCount,dyA,inverseAY),bitB=maskIndex(rowB.bitCount,dyB,inverseBY);
            if((rowA.words.at(bitA>>5)&(1U<<(bitA&31))) && (rowB.words.at(bitB>>5)&(1U<<(bitB&31)))) return true;
        }
    }
    return false;
}
}
