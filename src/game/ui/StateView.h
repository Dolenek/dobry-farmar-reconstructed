#pragma once
#include "MenuView.h"
#include "Control.h"
namespace farmar::recovered {
// Original common moving-scene View, 100 native bytes. Farm, warehouse and
// manufacture derive their real enter-state behavior from virtual slot +38.
class StateView:public MenuView {
public:
    StateView(View*,ResourceManager&,Settings&,StringManager&,ViewRegistry& views=globalViewRegistry(),ControlRegistry& controls=globalControlRegistry()); // 004c48e0
    ~StateView() override=default; // 0042c7d0 tail-calls MenuView destructor
    void loadXML(const XmlNode&) override; // 004c4a80
    void absoluteStep() override; // 004c56f0, actual scaled timer +8
    void move(unsigned state,const Point2& from,const Point2& to); // 004c5630; 1 outgoing, 2 incoming
    virtual void enterState() {} // Original base slot +38 is genuine RET 004bd820.
    void showOrigin(); // 004c4eb0
    void hideOrigin(); // 004c5270
    Picture& movingPicture() noexcept {return *picture_;}
    Control& origin() noexcept {return *origin_;}
    Point2 from{0,0},to{0,0},velocity{0,0};unsigned movement=0;
    float moveSpeed=2400,remaining=0; // Native constructor leaves +5c unspecified.
private:
    Picture* picture_;Control* origin_;
};
}
