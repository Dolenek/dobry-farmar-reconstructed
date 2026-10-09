#include "ControlSprite.h"
#include "OriginalArithmetic.h"
namespace farmar::recovered {
void updateControlSprite(SpriteDefinition& sprite,const ControlSpriteProperties& p,unsigned frame,float opacity) noexcept {
    const double columnSpan=originalDivide(1.0,originalUnsigned(p.columns));
    const float rowSpan=static_cast<float>(originalDivide(1.0,originalUnsigned(p.rows)));
    const double columnStart=originalMultiply(originalUnsigned(frame%p.columns),columnSpan);
    const double rowStart=originalMultiply(originalUnsigned(frame/p.columns),rowSpan);
    sprite.u0=static_cast<float>(originalAdd(columnStart,p.invertX ? columnSpan:0.0));
    sprite.u1=static_cast<float>(originalAdd(static_cast<float>(columnStart),p.invertX ? 0.0:columnSpan));
    sprite.v0=static_cast<float>(originalAdd(rowStart,p.invertY ? rowSpan:0.f));
    sprite.v1=static_cast<float>(originalAdd(static_cast<float>(rowStart),p.invertY ? 0.f:rowSpan));
    // The game uses native x87 FSIN/FCOS with a float degree-to-radian factor.
    // Keep that operation rather than relying on a different CRT's trig reducer.
    const float angle=p.angle,radians=0.01745329238474369f;
    float cosine; double sine;
    __asm {
        fld angle
        fmul radians
        fld st(0)
        fcos
        fstp cosine
        fsin
        fstp sine
    }
    Color color=p.color; color.a=static_cast<float>(originalMultiply(originalMultiply(p.rootAlpha,p.alpha),opacity));
    const auto packed=color.toARGB();
    for(unsigned i=0;i<4;++i) {
        const double w=originalMultiply(p.width,i&1 ? 0.5:-0.5);
        const float h=static_cast<float>(originalMultiply(p.height,i&2 ? 0.5:-0.5));
        const double x=originalAdd(originalMultiply(h,sine),originalMultiply(w,cosine));
        const float y=static_cast<float>(originalSubtract(originalMultiply(h,cosine),originalMultiply(w,sine)));
        sprite.corners[i]={static_cast<float>(originalAdd(i==0 ? x:static_cast<float>(x),p.x)),y+p.y,packed};
    }
}
}
