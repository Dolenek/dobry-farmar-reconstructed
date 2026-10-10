#pragma once
#include "ControlSprite.h"
namespace farmar::recovered {
// cProgressBar::makeSprite 004f9290. The native four side branches and float
// spills differ from cControl, including the observed side 3 top-edge mutation.
void updateProgressSprite(SpriteDefinition&,const ControlSpriteProperties&,unsigned frame,
    float maximum,float& current,unsigned side) noexcept;
}
