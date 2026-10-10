#include "Slider.h"
#include "XmlDocument.h"
#include "OriginalArithmetic.h"
#include <sstream>
#include <stdexcept>
namespace farmar::recovered {
namespace {
template<class T> void read(const XmlNode& node,const wchar_t* key,T& value) {
    const auto text=node.attribute(WString(key));if(!text.empty()) { std::wistringstream stream(text.c_str());stream>>value; }
}
}
SliderThumb::SliderThumb(Slider& owner,ResourceManager& resources,InputBindings& input,ButtonInteraction& interaction,ViewRegistry& views,ControlRegistry& controls)
    :Button(&owner,resources,input,interaction,views,controls),owner_(owner) {}
void SliderThumb::clicked() { ButtonPrototype::clicked();owner_.setDragging(true); }
void SliderThumb::unclicked() { ButtonPrototype::unclicked();owner_.setDragging(false); }
void SliderThumb::onHidden() { owner_.setDragging(false);Picture::onHidden(); }
Slider::Slider(View* parent,ResourceManager& resources,InputBindings& input,ButtonInteraction& interaction,ViewRegistry& views,ControlRegistry& controls)
    :Button(parent,resources,input,interaction,views,controls),thumb_(new SliderThumb(*this,resources,input,interaction,views,controls)) {}
void Slider::setDragging(bool value) { if(dragging_!=value) { dragging_=value;if(dragChanged) dragChanged(this,value); } }
void Slider::clicked() { ButtonPrototype::clicked();setDragging(true); }
void Slider::unclicked() { ButtonPrototype::unclicked();setDragging(false); }
void Slider::loadXML(const XmlNode& node) {
    Button::loadXML(node);XmlNode* entry=nullptr;if(node.section(String("Slider"),entry)) thumb_->loadXML(*entry);
    read(node,L"Max",maximum);read(node,L"Min",minimum);read(node,L"Value",value_);layout=0;read(node,L"Layout",layout);updateThumb();
}
void Slider::updateThumb() {
    float axisX=0,axisY=0,low=0,high=0;
    if(layout==0) { axisX=1;const auto half=originalMultiply(geometry_.width,.5);low=float(originalSubtract(geometry_.x,half));high=float(originalAdd(half,geometry_.x)); }
    else if(layout==1) { axisY=1;const auto half=originalMultiply(geometry_.height,.5);low=float(originalSubtract(geometry_.y,half));high=float(originalAdd(half,geometry_.y)); }
    const float halfX=float(originalMultiply(originalMultiply(axisX,geometry_.width),.5));
    const float halfY=float(originalMultiply(originalMultiply(axisY,geometry_.height),.5));
    const auto travel=originalSubtract(high,low);const float travelX=float(originalMultiply(travel,axisX));const auto travelY=originalMultiply(axisY,travel);
    const float delta=float(originalSubtract(value_,minimum));const float ratio=float(originalDivide(1,originalSubtract(maximum,minimum)));
    const float offsetY=float(originalMultiply(originalMultiply(travelY,delta),ratio));
    const auto offsetX=originalMultiply(originalMultiply(travelX,delta),ratio);
    const float baseX=float(originalSubtract(geometry_.x,halfX));const auto baseY=originalSubtract(geometry_.y,halfY);
    thumb_->setPosition({float(originalAdd(baseX,offsetX)),float(originalAdd(baseY,offsetY))});
}
void Slider::enabledStep() {
    if(!dragging_) return;
    if(!interaction_.cursor) throw std::logic_error("Slider cursor service is not bound");const auto cursor=interaction_.cursor();
    float axisX=0,axisY=0;double low=0,high=0;
    if(layout==0) { axisX=1;const float half=float(originalMultiply(geometry_.width,.5));low=originalSubtract(geometry_.x,half);high=originalAdd(half,geometry_.x); }
    else if(layout==1) { axisY=1;const float half=float(originalMultiply(geometry_.height,.5));low=originalSubtract(geometry_.y,half);high=originalAdd(half,geometry_.y); }
    const auto x=originalMultiply(cursor[0],axisX),y=originalMultiply(cursor[1],axisY);
    auto coordinate=originalSqrt(originalAdd(originalMultiply(x,x),originalMultiply(y,y)));
    if(coordinate<low) coordinate=low;if(high<coordinate) coordinate=high;
    const float next=float(originalAdd(originalDivide(originalMultiply(originalSubtract(coordinate,low),originalSubtract(maximum,minimum)),originalSubtract(high,low)),minimum));
    if(next!=value_) { value_=next;if(valueChanged) valueChanged(this,next);updateThumb(); }
}
}
