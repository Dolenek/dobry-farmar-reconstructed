// Diagnostic metadata only: no game construction, original calls or mutation.
#include "TutorialManager.h"
#include <cstddef>
#include <iostream>
using namespace farmar::recovered;
struct ControlLayout:Control {
    static auto geometryOffset() {return offsetof(ControlLayout,geometry_);}
    static auto stateOffset() {return offsetof(ControlLayout,state_);}
    static auto spriteOffset() {return offsetof(ControlLayout,first_);}
    // MSVC /d1reportSingleClassLayoutControl: controls_ is a four-byte
    // reference immediately before geometry_. offsetof(reference) yields 0.
    static auto registryOffset() {return offsetof(ControlLayout,geometry_)-sizeof(void*);}
    static auto deepOffset() {return offsetof(ControlLayout,deep_);}
    static auto flagsOffset() {return offsetof(ControlLayout,flags_);}
    static auto textureOffset() {return offsetof(ControlLayout,texture_);}
    static auto alphaOffset() {return offsetof(ControlLayout,alpha_);}
};
struct TipLayout:TutorialActionTip {
    static auto imageOffset() {return offsetof(TipLayout,image_);}
    static auto textOffset() {return offsetof(TipLayout,text_);}
    static auto okOffset() {return offsetof(TipLayout,ok_);}
};
int main() {
    std::cout<<"{\"manager_size\":"<<sizeof(TutorialManager)
        <<",\"current_step\":"<<offsetof(TutorialManager,currentStep)
        <<",\"control_state\":"<<ControlLayout::stateOffset()
        <<",\"geometry\":"<<ControlLayout::geometryOffset()
        <<",\"sprite\":"<<ControlLayout::spriteOffset()
        <<",\"registry\":"<<ControlLayout::registryOffset()
        <<",\"deep\":"<<ControlLayout::deepOffset()
        <<",\"flags\":"<<ControlLayout::flagsOffset()
        <<",\"texture\":"<<ControlLayout::textureOffset()
        <<",\"alpha\":"<<ControlLayout::alphaOffset()
        <<",\"image\":"<<TipLayout::imageOffset()
        <<",\"text\":"<<TipLayout::textOffset()
        <<",\"ok\":"<<TipLayout::okOffset()<<"}\n";
}
