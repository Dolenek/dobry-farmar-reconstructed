#include "ElasticAnimation.h"
#include "DigBuilding.h"
#include "MowBuilding.h"
#include "WaterSource.h"
#include "SoundManager.h"
#include "Sound.h"
#include "ResourceManager.h"
#include "XmlDocument.h"
#include "StringConversion.h"
#include "OriginalArithmetic.h"
#include <algorithm>
#include <sstream>
#include <stdexcept>
namespace farmar::recovered {
namespace {
ElasticAnimation* elasticInstance=nullptr;
void number(float& value,const XmlNode& node,const wchar_t* name) {const auto text=node.attribute(WString(name));if(!text.empty()) {std::wistringstream input(text.c_str());input>>value;}}
}
ElasticAnimationLifetime::ElasticAnimationLifetime(ElasticAnimation* value) {if(elasticInstance) throw "Object already exist.";elasticInstance=value;}
ElasticAnimationLifetime::~ElasticAnimationLifetime() {elasticInstance=nullptr;}
ElasticAnimation* ElasticAnimation::instance() noexcept {return elasticInstance;}
ElasticAnimation::ElasticAnimation(View* parent,ResourceManager& resources,SoundManager& sound,ViewRegistry& views,ControlRegistry& controls)
    :Picture(parent,resources,views,controls),ElasticAnimationLifetime(this),sound_(sound) {loadXML(*resources.loadSettingsXML(String("Farm/ElasticAnimation.xml")));}
ElasticAnimation::~ElasticAnimation() {if(moveSound_) moveSound_->releaseReference();}
void ElasticAnimation::loadXML(const XmlNode& node) {
    Picture::loadXML(node);number(rotationSpeed,node,L"RotationSpeed");number(speed,node,L"Speed");number(acceleration,node,L"Accelerate");
    const auto path=toNarrow(node.attribute(WString(L"MoveSound")));if(!path.empty()) moveSound_=resources_.getSound(path);
    XmlNode* list=nullptr;if(node.section(String("States"),list)) for(unsigned i=0;i<(std::min)(10U,unsigned(list->children().size()));++i) {XmlNode* part=nullptr;if(list->section(i,part)) states[i].loadXML(*part,resources_);}
}
void ElasticAnimation::begin(const Point2& from,const Point2& to,int value,View* reveal) {
    if(from[0]==to[0] && from[1]==to[1]) return;
    rotation=0;velocity={float(originalSubtract(to[0],from[0])),float(originalSubtract(to[1],from[1]))};
    // 0042b83f stores without pop: normalized X uses the unspilled inverse,
    // Y reads the float coefficient0065ff94.
    const auto inverse=originalDivide(1,originalSqrt(originalAdd(originalMultiply(velocity[0],velocity[0]),originalMultiply(velocity[1],velocity[1]))));
    const float coefficient=float(inverse);velocity={float(originalMultiply(inverse,velocity[0])),float(originalMultiply(coefficient,velocity[1]))};
    accelerationVector={float(originalMultiply(acceleration,velocity[0])),float(originalMultiply(acceleration,velocity[1]))};
    velocity={float(originalMultiply(speed,velocity[0])),float(originalMultiply(speed,velocity[1]))};kind=value;revealView=reveal;
    const auto dx=originalSubtract(to[0],from[0]),dy=originalSubtract(to[1],from[1]);
    const auto distance=originalSqrt(originalAdd(originalMultiply(dx,dx),originalMultiply(dy,dy)));
    const auto force=originalMultiply(distance,acceleration);
    remaining=float(originalDivide(originalSubtract(originalSqrt(originalAdd(originalAdd(force,force),originalMultiply(speed,speed))),speed),acceleration));
    const auto& appearance=states.at(unsigned(kind));if(texture()!=appearance.texture) {copyAppearance(appearance);markTransformDirty();}
    position=from;setPosition(position);if(moveSound_) sound_.clickSound(static_cast<cSound*>(moveSound_));show();
}
void ElasticAnimation::absoluteStep() {
    if(remaining==0) return;const float dt=uiElapsedTime()();remaining=float(originalSubtract(remaining,dt));rotation=float(originalAdd(originalMultiply(dt,rotationSpeed),rotation));setAngle(rotation);
    if(!(remaining<=0)) { // native zero<remaining also advances unordered values
        position={float(originalAdd(originalMultiply(dt,velocity[0]),geometry_.x)),float(originalAdd(originalMultiply(dt,velocity[1]),geometry_.y))};
        velocity={float(originalAdd(originalMultiply(dt,accelerationVector[0]),velocity[0])),float(originalAdd(originalMultiply(dt,accelerationVector[1]),velocity[1]))};setPosition(position);return;
    }
    remaining=0;hide();if(kind==9) {if(!revealView) throw std::logic_error("Elastic kind9 requires original reveal View");revealView->show();}
    if(kind>=1 && kind<5) {auto* dig=DigBuilding::instance();if(!dig) throw std::logic_error("Elastic completion requires actual DigBuilding");dig->refreshAppearance();}
    if(kind>4 && kind<9) {auto* mow=MowBuilding::instance();if(!mow) throw std::logic_error("Elastic completion requires actual MowBuilding");mow->refreshAppearance();}
    if(kind==0) {auto* water=WaterSource::instance();if(!water) throw std::logic_error("Elastic completion requires actual WaterSource");water->refreshAppearance();}
}
}
