#pragma once
#include "Control.h"
#include <vector>
namespace farmar::recovered {
// Actual common Picture base of BuySeeds and BuyBuilding, native 636 bytes.
// Its +248 field is movement state, also read by tutorial predicates 13/32.
class MovableUnion:public Picture {
public:
    MovableUnion(View*,ResourceManager&,ViewRegistry& views=globalViewRegistry(),ControlRegistry& controls=globalControlRegistry()); // 00413780
    ~MovableUnion() override=default; // 00413fa0: list entries then Picture/View ownership
    void loadXML(const XmlNode&) override; // 00414d60
    void absoluteStep() override; // 004148e0
    void move(unsigned,const Point2& from,const Point2& to); // 004147f0
    void riseControls(); // 00414070, shows the actual origin control
    void unriseControls(); // 00414430, hides the actual origin control
    Control& origin() noexcept {return *origin_;}
    const std::vector<Picture*>& backgrounds() const noexcept {return backgrounds_;}
    Point2 from{},to{},velocity{},hiddenPlace{},originalPosition{};
    unsigned movement=0;float moveSpeed=0,remaining=0; // native speed/remaining unspecified until assigned
private:
    Control* origin_;std::vector<Picture*> backgrounds_;
};
}
