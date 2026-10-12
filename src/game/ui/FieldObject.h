#pragma once
#include "Button.h"
namespace farmar::recovered {
// Original cFieldObject<cAnimationButton>: AnimationButton plus the float at
// +27c, preceding the ActionArea tooltip specialization. Modern source layout
// is intentionally separate from the diagnostic native ABI.
class FieldObject:public AnimationButton {
public:
    using AnimationButton::AnimationButton; // base ctor 004f1c30; +27c initialized in 0041be20
    void loadXML(const XmlNode&) override; // 00419680
    void setPosition(const Point2&) override; // 004192a0
    void updateTransform() override; // 00419a90
    bool pointInRect(const Point2&) const override; // 00417fe0; original mutates/restores y by float operations
    float heightOffset=0;
};
}
