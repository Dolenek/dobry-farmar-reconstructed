#include "FlyingText.h"
#include "ResourceManager.h"
#include "XmlDocument.h"
#include "OriginalArithmetic.h"
#include <sstream>
#include <stdexcept>
namespace farmar::recovered {
namespace {
template<class T> void attribute(const XmlNode& node,const wchar_t* name,T& value) {const auto text=node.attribute(WString(name));if(!text.empty()) {std::wistringstream input(text.c_str());input>>value;}}
void position(const XmlNode& node,Point2& value) {const auto text=node.attribute(WString(L"position"));if(!text.empty()) {std::wistringstream input(text.c_str());input>>value[0];input.ignore(1);input>>value[1];}}
}
FlyingText::FlyingText(View* parent,FlyingTextContext& c,const String& xml):TextLine(parent,c.resources,c.settings,c.strings,c.views,c.controls) {
    if(!xml.empty()) loadXML(*c.resources.loadSettingsXML(xml));
}
void FlyingText::loadXML(const XmlNode& node) {
    TextLine::loadXML(node);icon_=new Picture(this,resources_,registry_,controls_);
    XmlNode* definition=nullptr;if(node.section(String("Icon"),definition)) icon_->loadXML(*definition);
    attribute(node,L"Speed",speed);
    float sum=0;XmlNode* list=nullptr;
    if(node.section(String("PositionPoints"),list)) {
        positions_.clear();positions_.resize(list->children().size(),FlyingPosition{});
        for(unsigned i=0;i<positions_.size();++i) {
            XmlNode* entry=nullptr;if(!list->section(i,entry)) continue;
            auto& point=positions_[i];position(*entry,point.position);
            if(!i) continue;auto& previous=positions_[i-1];
            const auto dx=originalSubtract(point.position[0],previous.position[0]),dy=originalSubtract(point.position[1],previous.position[1]);const float spilledY=float(dy);
            const auto length=originalSqrt(originalAdd(originalMultiply(dy,spilledY),originalMultiply(dx,dx)));
            sum=float(originalAdd(sum,length));point.distance=sum;
            const float inverse=float(originalDivide(1,length));
            previous.velocity={float(originalMultiply(dx,inverse)),float(originalMultiply(spilledY,inverse))};
        }
    }
    if(positions_.size()<2) throw "Position points < 2";
    if(sum==0) throw "Distance = 0";
    totalDistance=sum;setPosition(positions_.front().position);
    if(node.section(String("AlphaPoints"),list)) {
        alphas_.clear();alphas_.resize(list->children().size(),FlyingAlpha{});
        for(unsigned i=0;i<alphas_.size();++i) {
            XmlNode* entry=nullptr;if(!list->section(i,entry)) continue;
            auto& point=alphas_[i];attribute(*entry,L"alpha",point.alpha);attribute(*entry,L"distance",point.distance);
            if(i) alphas_[i-1].delta=float(originalSubtract(point.alpha,alphas_[i-1].alpha));
        }
    }
    if(alphas_.size()<2) throw "Alpha points < 2";
    setAlpha(alphas_.front().alpha);textDirty_=true;icon_->setAlpha(alphas_.front().alpha);icon_->markTransformDirty();
    // The native loader does not reset distance, indices, or completion.
    // Lifespan is not read by cFlyText.
}
void FlyingText::setIconAppearance(const AnimableAppearance& appearance) {icon_->copyAppearance(appearance);icon_->markTransformDirty();}
void FlyingText::finish() {
    setPosition(positions_.back().position);setAlpha(alphas_.back().alpha);
    if(completed) {completed(completionArgument);completed=nullptr;}
    registry_.requestDiscard(this,String()); // original 0047d0b0 / 00432b20
}
void FlyingText::enabledStep() {
    TextLine::enabledStep();if(!uiElapsedTime()) throw std::logic_error("FlyingText timer is not bound");
    const float elapsed=uiElapsedTime()();const auto next=originalAdd(originalMultiply(elapsed,speed),distance);distance=float(next);
    if(next<totalDistance) {
        while(positionIndex<positions_.size() && distance>=positions_[positionIndex].distance) ++positionIndex;
        if(positionIndex<positions_.size()) {
            const auto& previous=positions_[positionIndex-1];const float delta=float(originalSubtract(distance,previous.distance));
            const float dy=float(originalMultiply(delta,previous.velocity[1]));
            setPosition({float(originalAdd(originalMultiply(delta,previous.velocity[0]),previous.position[0])),float(originalAdd(dy,previous.position[1]))});
        }else finish();
    }else finish();
    const auto ratio=originalDivide(distance,totalDistance);
    while(alphaIndex<alphas_.size() && ratio>=alphas_[alphaIndex].distance) ++alphaIndex;
    float alpha=alphas_.back().alpha;
    if(alphaIndex<alphas_.size()) {
        const auto& previous=alphas_[alphaIndex-1];
        alpha=float(originalAdd(originalMultiply(originalDivide(originalSubtract(ratio,previous.distance),originalSubtract(alphas_[alphaIndex].distance,previous.distance)),previous.delta),previous.alpha));
        setAlpha(alpha);
    }
    icon_->setAlpha(alpha);icon_->markTransformDirty();textDirty_=true;
}
}
