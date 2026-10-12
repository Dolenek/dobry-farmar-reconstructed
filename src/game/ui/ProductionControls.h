#pragma once
#include "Button.h"
#include "AnimableAppearance.h"
#include <array>
namespace farmar::recovered {
class ProductionBuilding;
// 00499ad0: 720 native bytes, AnimationButton + owner + two appearances.
// The building installs its real stop-toggle callback after loading its XML.
class ProductionStartStopButton final:public AnimationButton {
public:
    ProductionStartStopButton(View*,ProductionBuilding*,ResourceManager&,InputBindings&,ButtonInteraction&,
        ViewRegistry& views=globalViewRegistry(),ControlRegistry& controls=globalControlRegistry());
    void loadXML(const XmlNode&) override; // 0049ab60
    void setStoppedSign(unsigned char); // 00499b90, byte == 1 selects StartSign; no profile/dirty change
    ProductionBuilding* owner() const noexcept {return owner_;}
    const std::array<AnimableAppearance,2>& signs() const noexcept {return signs_;}
private:
    ProductionBuilding* owner_;std::array<AnimableAppearance,2> signs_; // native +27c, +280/+2a8
};
}
