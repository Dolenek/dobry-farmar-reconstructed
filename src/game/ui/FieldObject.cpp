#include "FieldObject.h"
#include "XmlDocument.h"
#include "OriginalArithmetic.h"
#include <sstream>
namespace farmar::recovered {
void FieldObject::loadXML(const XmlNode& node) {
    AnimationButton::loadXML(node);
    const auto text=node.attribute(WString(L"HeightOffset"));
    if(!text.empty()) {std::wistringstream stream(text.c_str());stream>>heightOffset;}
}
void FieldObject::setPosition(const Point2& position) {
    Control::setPosition(position);
    setDeep(float(originalAdd(heightOffset,position[1])));
}
void FieldObject::updateTransform() {
    // 00419a90 spills both operations, even when the base transform is already
    // clean. Preserve subtraction/addition rounding rather than restoring a
    // saved y value; the original may drift after repeated offset transforms.
    geometry_.y=float(originalSubtract(geometry_.y,heightOffset));
    Control::updateTransform();
    geometry_.y=float(originalAdd(heightOffset,geometry_.y));
}
bool FieldObject::pointInRect(const Point2& point) const {
    auto& self=*const_cast<FieldObject*>(this);
    self.geometry_.y=float(originalSubtract(self.geometry_.y,heightOffset));
    const bool result=ButtonPrototype::pointInRect(point);
    self.geometry_.y=float(originalAdd(heightOffset,self.geometry_.y));
    return result;
}
}
