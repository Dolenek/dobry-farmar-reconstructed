#include "MovableUnion.h"
#include "XmlDocument.h"
#include "OriginalArithmetic.h"
#include <sstream>
#include <stdexcept>
namespace farmar::recovered {
MovableUnion::MovableUnion(View* parent,ResourceManager& resources,ViewRegistry& views,ControlRegistry& controls)
    :Picture(parent,resources,views,controls),origin_(new Control(this,resources,views,controls)) {
    origin_->setPosition({0,0});origin_->setSize({1,1});origin_->show();origin_->enable();origin_->absoluteEnable();
}
void MovableUnion::loadXML(const XmlNode& node) {
    Picture::loadXML(node);XmlNode* section=nullptr;
    if(node.section(String("Background"),section)) for(unsigned i=0;i<section->children().size();++i) {XmlNode* item=nullptr;if(section->section(i,item)) {auto* picture=new Picture(this,resources_,registry_,controls_);picture->loadXML(*item);backgrounds_.push_back(picture);}}
    const auto speed=node.attribute(WString(L"MoveSpeed"));if(!speed.empty()) {std::wistringstream in(speed.c_str());in>>moveSpeed;}
    const auto hidden=node.attribute(WString(L"HiddenPlace"));if(!hidden.empty()) {std::wistringstream in(hidden.c_str());in>>hiddenPlace[0];in.ignore(1);in>>hiddenPlace[1];}
    originalPosition={geometry().x,geometry().y};
}
void MovableUnion::riseControls() {origin_->show();}
void MovableUnion::unriseControls() {origin_->hide();}
void MovableUnion::move(unsigned value,const Point2& start,const Point2& target) {
    movement=value;if(!value) return;from=start;to=target;
    const auto dx=originalSubtract(to[0],from[0]),dy=originalSubtract(to[1],from[1]);
    const auto duration=originalDivide(originalSqrt(originalAdd(originalMultiply(dy,dy),originalMultiply(dx,dx))),moveSpeed);
    remaining=float(duration);const float inverse=float(originalDivide(1,duration));
    velocity={float(originalMultiply(inverse,float(dx))),float(originalMultiply(inverse,float(dy)))};
    setPosition(from);show();disable();unriseControls();
}
void MovableUnion::absoluteStep() {
    View::absoluteStep();if(!uiElapsedTime()) throw std::logic_error("UI timer is not bound");const float elapsed=uiElapsedTime()();
    if(!movement) return;const auto left=originalSubtract(remaining,elapsed);remaining=float(left);
    if(!(left<=0)) setPosition({float(originalAdd(originalMultiply(elapsed,velocity[0]),geometry().x)),float(originalAdd(geometry().y,originalMultiply(elapsed,velocity[1])))});
    else {setPosition(to);if(movement==1) hide();riseControls();enable();movement=0;}
}
}
