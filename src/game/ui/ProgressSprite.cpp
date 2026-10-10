#include "ProgressSprite.h"
#include "OriginalArithmetic.h"
#include <initializer_list>
namespace farmar::recovered {
void updateProgressSprite(SpriteDefinition& sprite,const ControlSpriteProperties& p,unsigned frame,
    float maximum,float& current,unsigned side) noexcept {
    const float dx=static_cast<float>(originalDivide(1,originalUnsigned(p.columns)));
    const float dy=static_cast<float>(originalDivide(1,originalUnsigned(p.rows)));
    sprite.u0=static_cast<float>(originalMultiply(originalUnsigned(frame%p.columns),dx));
    sprite.v0=static_cast<float>(originalMultiply(originalUnsigned(frame/p.columns),dy));
    sprite.u1=static_cast<float>(originalAdd(sprite.u0,dx));sprite.v1=static_cast<float>(originalAdd(sprite.v0,dy));
    for(unsigned i=0;i<4;++i) {
        sprite.corners[i].x=static_cast<float>(originalMultiply(p.width,i&1 ? .5:-.5));
        sprite.corners[i].y=static_cast<float>(originalMultiply(p.height,i&2 ? .5:-.5));
    }
    if(current>maximum) current=maximum;
    if(current<0) current=0;
    const float fraction=static_cast<float>(originalDivide(current,maximum));
    const double missing=originalSubtract(1,fraction);
    switch(side) {
    case 0:
        sprite.u0=static_cast<float>(originalAdd(sprite.u0,originalMultiply(dx,missing)));
        for(unsigned i:{0U,2U}) sprite.corners[i].x=static_cast<float>(originalAdd(sprite.corners[i].x,originalMultiply(p.width,missing)));
        break;
    case 1:
        sprite.u1=static_cast<float>(originalSubtract(sprite.u1,originalMultiply(dx,missing)));
        for(unsigned i:{1U,3U}) sprite.corners[i].x=static_cast<float>(originalSubtract(sprite.corners[i].x,originalMultiply(p.width,missing)));
        break;
    case 2:
        sprite.v0=static_cast<float>(originalAdd(sprite.v0,originalMultiply(dy,missing)));
        for(unsigned i:{0U,1U}) sprite.corners[i].y=static_cast<float>(originalAdd(sprite.corners[i].y,originalMultiply(p.height,missing)));
        break;
    case 3:
        sprite.v1=static_cast<float>(originalSubtract(sprite.v1,originalMultiply(dy,missing)));
        for(unsigned i:{0U,1U}) sprite.corners[i].y=static_cast<float>(originalSubtract(sprite.corners[i].y,originalMultiply(p.height,missing)));
        break;
    }
    const float angle=p.angle,radians=.01745329238474369f;
    float cosine;
    __asm {
        fld angle
        fmul radians
        fcos
        fstp cosine
    }
    Color color=p.color;color.a=static_cast<float>(originalMultiply(p.rootAlpha,p.alpha));
    const auto packed=color.toARGB();
    for(auto& corner:sprite.corners) {
        const float x=corner.x,y=corner.y,px=p.x,py=p.y;
        float rotatedX,outX,outY;
        // FSIN remains in the x87 register exactly as in the native routine;
        // rotated X is spilled to binary32 before adding the control position.
        __asm {
            fld angle
            fmul radians
            fsin
            fld st(0)
            fchs
            fmul x
            fld cosine
            fmul y
            faddp st(1),st(0)
            fadd py
            fstp outY
            fmul y
            fld cosine
            fmul x
            faddp st(1),st(0)
            fstp rotatedX
            fld rotatedX
            fadd px
            fstp outX
        }
        corner={outX,outY,packed};
    }
}
}
