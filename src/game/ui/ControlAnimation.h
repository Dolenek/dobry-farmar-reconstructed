#pragma once
#include "ControlSprite.h"
namespace farmar::recovered {
struct ControlAnimationState {
    float time=0,length=1;
    unsigned frames=1;
    bool loop=false,doubleRender=false;
};
struct ControlAnimationFrame {
    unsigned first=0,second=0,count=1;
    float firstOpacity=1,secondOpacity=0;
    bool ended=false;
};
float normalizeControlAnimationTime(float time,float length,bool loop) noexcept; // 004ed510
unsigned controlAnimationFrame(float time,float length,unsigned frames) noexcept; // 00413a10
// 2D animation arithmetic in 004f0b70; the caller dispatches its ended callback.
ControlAnimationFrame resolveControlAnimation(ControlAnimationState& animation) noexcept;
void updateAnimatedControlSprites(SpriteDefinition& first,SpriteDefinition& second,
    const ControlSpriteProperties& properties,const ControlAnimationFrame& frame) noexcept;
}
