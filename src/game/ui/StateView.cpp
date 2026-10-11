#include "StateView.h"
#include "XmlDocument.h"
#include "OriginalArithmetic.h"
#include <sstream>
#include <stdexcept>
namespace farmar::recovered {
StateView::StateView(View* parent,ResourceManager& resources,Settings& settings,StringManager& strings,ViewRegistry& views,ControlRegistry& controls)
    :MenuView(parent,resources,settings,strings,views,controls),picture_(new Picture(this,resources,views,controls)),origin_(new Control(picture_,resources,views,controls)) {
    picture_->setPosition({-1,0});picture_->setSize({1,1});picture_->show();picture_->enable();picture_->absoluteEnable();
    origin_->setPosition({0,0});origin_->setSize({1,1});origin_->show();origin_->enable();origin_->absoluteEnable();
}
void StateView::loadXML(const XmlNode& node) {
    MenuView::loadXML(node);for(auto* item:decorations()) item->setParent(picture_);
    const auto speed=node.attribute(WString(L"MoveSpeed"));if(!speed.empty()) {std::wistringstream stream(speed.c_str());stream>>moveSpeed;}
}
void StateView::showOrigin() {origin_->show();}
void StateView::hideOrigin() {origin_->hide();}
void StateView::move(unsigned state,const Point2& start,const Point2& target) {
    movement=state;if(!state) return;from=start;to=target;
    // Squared deltas and duration remain in x87 registers. The reciprocal
    // spills independently; velocity multiplies float-spilled deltas.
    const auto dx=originalSubtract(to[0],from[0]),dy=originalSubtract(to[1],from[1]);
    const auto duration=originalDivide(originalSqrt(originalAdd(originalMultiply(dy,dy),originalMultiply(dx,dx))),moveSpeed);
    remaining=float(duration);const float inverse=float(originalDivide(1.0,duration));
    velocity={float(originalMultiply(inverse,float(dx))),float(originalMultiply(inverse,float(dy)))};
    picture_->setPosition(from);show();disable();hideOrigin();
}
void StateView::absoluteStep() {
    View::absoluteStep();if(!uiElapsedTime()) throw std::logic_error("UI timer is not bound");const float elapsed=uiElapsedTime()();
    if(!movement) return;const auto left=originalSubtract(remaining,elapsed);remaining=float(left);
    // Native compares zero < unspilled left by x87 C0; unordered also advances.
    if(!(left<=0)) {
        picture_->setPosition({float(originalAdd(originalMultiply(elapsed,velocity[0]),picture_->geometry().x)),float(originalAdd(picture_->geometry().y,originalMultiply(elapsed,velocity[1])))});
    } else {
        picture_->setPosition(to);if(movement==1) hide();else enterState();showOrigin();
        if(!state().enabled || !state().parentEnabled) enable();movement=0;
    }
}
}
