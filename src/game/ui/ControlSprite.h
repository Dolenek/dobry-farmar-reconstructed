#pragma once
#include "SpriteGeometry.h"
#include "GameColor.h"
namespace farmar::recovered {
struct ControlSpriteProperties {
    float x=0,y=0,width=1,height=1,angle=0;
    unsigned columns=1,rows=1;
    bool invertX=false,invertY=false;
    Color color{1,1,1,1};
    float rootAlpha=1,alpha=1;
};
// cControl::makeSprite normal path, 004ef3b0. The third original stack argument
// is an opacity float omitted by the automatic decompiler signature.
void updateControlSprite(SpriteDefinition& sprite,const ControlSpriteProperties& properties,
    unsigned frame,float opacity=1) noexcept;
}
