// Read-only compiler layout metadata. No game construction or callbacks.
#include "TutorialManager.h"
#include "Button.h"
#include <cstddef>
#include <iostream>
using namespace farmar::recovered;
struct ButtonLayout:ButtonPrototype {
    static auto stateOffset() {return offsetof(ButtonLayout,buttonState_);}
};
int main() {
    std::cout<<"{\"manager_size\":"<<sizeof(TutorialManager)
        <<",\"elapsed\":"<<offsetof(TutorialManager,elapsed)
        <<",\"capacity_previous\":"<<offsetof(TutorialManager,valueC10)
        <<",\"capacity_counter\":"<<offsetof(TutorialManager,valueC14)
        <<",\"mixed_previous\":"<<offsetof(TutorialManager,valueC18)
        <<",\"mixed_counter\":"<<offsetof(TutorialManager,valueC1c)
        <<",\"button_state\":"<<ButtonLayout::stateOffset()<<"}\n";
}
