#include "WaterEmitters.h"
#include "GameRandom.h"
#include "ResourceManager.h"
#include "XmlDocument.h"
#include "TextLine.h"
#include "OriginalArithmetic.h"
#include <cmath>
#include <sstream>
#include <stdexcept>
namespace farmar::recovered {
namespace {
void number(float& value,const XmlNode& node,const wchar_t* key) {const auto text=node.attribute(WString(key));if(!text.empty()) {std::wistringstream stream(text.c_str());stream>>value;}}
void pair(Point2& value,const XmlNode& node,const wchar_t* key) {const auto text=node.attribute(WString(key));if(!text.empty()) {std::wistringstream stream(text.c_str());stream>>value[0]>>value[1];}}
float frequency(const XmlNode& node) {const auto text=node.attribute(WString(L"Frequency"));if(text.empty()) throw std::logic_error("Water emitter Frequency is missing; original unspecified stack input is not recovered");float value;std::wistringstream stream(text.c_str());stream>>value;return value;}
double interpolate(float low,float high,int random) {return originalAdd(originalMultiply(originalMultiply(originalSubtract(high,low),random),3.0518509447574615e-05f),low);}
}
WaterEmitter::WaterEmitter(View* parent,ResourceManager& resources,GameRandom& random,ViewRegistry& views,ControlRegistry& controls):Control(parent,resources,views,controls),random_(random) {emission.interval=float(originalDivide(1,15));}
WaterEmitter::~WaterEmitter() {for(auto* particle:particles_) registry_.requestDiscard(particle,String("d:\\work\\farmer\\src\\farmer\\game\\ParticleSystemMy.inl56"));}
void WaterEmitter::start(float duration) {emission.mode=1;emission.infinite=duration==0;emission.generating=true;emission.timeLimit=duration;emission.accumulated=emission.interval;emission.timeGenerated=0;}
void WaterEmitter::stepEmitter(const String& origin) {
    auto& s=emission;
    if(s.generating) {
        if(!uiElapsedTime()) throw std::logic_error("Water emitter timer is not bound");s.accumulated=float(originalAdd(uiElapsedTime()(),s.accumulated));
        if(s.infinite) {
            while(s.interval<=s.accumulated) {s.accumulated=float(originalSubtract(s.accumulated,s.interval));if(auto* particle=emitParticle()) particles_.push_back(particle);}
        }else if(s.mode==0) {
            do {
                Picture* particle;
                do {if(s.accumulated<s.interval) goto doneGenerating;particle=emitParticle();}while(!particle);
                particles_.push_back(particle);s.accumulated=float(originalSubtract(s.accumulated,s.interval));++s.createdCount;
            }while(s.createdCount<s.maximumCount);s.generating=false;
        }else if(s.mode==1) {
            while(s.interval<=s.accumulated) {
                if(auto* particle=emitParticle()) particles_.push_back(particle);s.accumulated=float(originalSubtract(s.accumulated,s.interval));const auto next=originalAdd(s.timeGenerated,s.interval);s.timeGenerated=float(next);if(s.timeLimit<=next) s.generating=false;
            }
        }
    }
doneGenerating:
    for(auto it=particles_.begin();it!=particles_.end();) {auto* particle=*it;if(particleFinished(*particle)) {it=particles_.erase(it);registry_.requestDiscard(particle,origin);}else ++it;}
}
void rotateWaterVector(Point2& vector,float angle) {
    constexpr float radians=0.01745329238474369f;
#if defined(_MSC_VER) && defined(_M_IX86)
    float cosine,negativeSine;float* data=vector.data();
    __asm {
        mov eax,data
        fld radians
        fmul angle
        fld st(0)
        fcos
        fstp cosine
        fsin
        fld st(0)
        fchs
        fstp negativeSine
        fmul dword ptr [eax+4]
        fld cosine
        fmul dword ptr [eax]
        faddp st(1),st(0)
        fld cosine
        fmul dword ptr [eax+4]
        fld negativeSine
        fmul dword ptr [eax]
        faddp st(1),st(0)
        fxch st(1)
        fstp dword ptr [eax]
        fstp dword ptr [eax+4]
    }
#else
    const auto a=originalMultiply(radians,angle);const float c=float(std::cos(a)),ns=float(-std::sin(a));const auto x=vector[0],y=vector[1];vector={float(originalAdd(originalMultiply(c,x),originalMultiply(std::sin(a),y))),float(originalAdd(originalMultiply(ns,x),originalMultiply(c,y)))};
#endif
}
HoseEmitter::HoseEmitter(View* parent,ResourceManager& resources,GameRandom& random,const String& path,ViewRegistry& views,ControlRegistry& controls):WaterEmitter(parent,resources,random,views,controls) {prototype_=new HoseParticle(this,resources,views,controls);loadXML(*resources.loadSettingsXML(path));prototype_->absoluteDisable();prototype_->disable();prototype_->hide();}
void HoseEmitter::loadXML(const XmlNode& node) {View::loadXML(node);prototype_->loadXML(node);pair(speed,node,L"Speed");number(distortionRadius,node,L"DistortionRadius");emission.interval=float(originalDivide(1,frequency(node)));}
bool HoseEmitter::particleFinished(const Picture& particle) const {return static_cast<const HoseParticle&>(particle).motion.finished;}
void HoseEmitter::absoluteStep() {stepEmitter(String(".\\Game\\Farm\\HoseEffect.cpp206"));}
HoseParticle* HoseEmitter::createParticle() {
    if(startPoint==targetPoint) return nullptr;
    auto* particle=new HoseParticle(*prototype_);Point2 distortion{0,distortionRadius};rotateWaterVector(distortion,float(originalMultiply(random_.next(),0.010986663f)));
    const Point2 target{float(originalAdd(distortion[0],targetPoint[0])),float(originalAdd(distortion[1],targetPoint[1]))};
    const float dx=float(originalSubtract(target[0],startPoint[0])),dy=float(originalSubtract(target[1],startPoint[1]));const float square=float(originalAdd(originalMultiply(dy,dy),originalMultiply(dx,dx)));
    const auto length=originalSqrt(square);const auto magnitude=interpolate(speed[0],speed[1],random_.next());const float duration=float(originalDivide(length,magnitude));
    const auto inverseDuration=originalDivide(1,duration);const float inverseDurationStored=float(inverseDuration),velocityMagnitude=float(originalMultiply(inverseDuration,length));
    const auto inverseLength=originalDivide(1,originalSqrt(square));Point2 normal{float(originalMultiply(inverseLength,dx)),float(originalMultiply(dy,float(inverseLength)))};rotateWaterVector(normal,-angle);
    const Point2 velocity{float(originalMultiply(originalSubtract(target[0],startPoint[0]),inverseDurationStored)),float(originalMultiply(normal[1],velocityMagnitude))};
    const auto difference=originalSubtract(originalSubtract(target[1],startPoint[1]),originalMultiply(velocity[1],duration));const float gravity=float(originalDivide(originalAdd(difference,difference),originalMultiply(duration,duration)));
    particle->motion.lifetime=duration;particle->motion.velocity=velocity;particle->gravity=gravity;
    particle->setPosition({float(originalAdd(startPoint[0],geometry_.x)),float(originalAdd(geometry_.y,startPoint[1]))});particle->show();particle->enable();particle->absoluteEnable();return particle;
}
SprinkleEmitter::SprinkleEmitter(View* parent,ResourceManager& resources,GameRandom& random,const String& path,ViewRegistry& views,ControlRegistry& controls):WaterEmitter(parent,resources,random,views,controls) {prototype_=new SprinkleParticle(this,resources,random,views,controls);loadXML(*resources.loadSettingsXML(path));prototype_->absoluteDisable();prototype_->disable();prototype_->hide();}
void SprinkleEmitter::loadXML(const XmlNode& node) {View::loadXML(node);prototype_->loadXML(node);number(randomAngle,node,L"RandAngle");pair(lifetimeRange,node,L"LiveTime");emission.interval=float(originalDivide(1,frequency(node)));}
bool SprinkleEmitter::particleFinished(const Picture& particle) const {return static_cast<const SprinkleParticle&>(particle).motion.finished;}
void SprinkleEmitter::absoluteStep() {stepEmitter(String(".\\Game\\Farm\\SprinkleEffect.cpp190"));}
SprinkleParticle* SprinkleEmitter::createParticle() {
    auto* particle=new SprinkleParticle(*prototype_);auto velocity=prototype_->motion.velocity;rotateWaterVector(velocity,float(interpolate(-randomAngle,randomAngle,random_.next())));
    particle->motion.lifetime=float(interpolate(lifetimeRange[0],lifetimeRange[1],random_.next()));particle->motion.velocity=velocity;particle->setPosition({geometry_.x,geometry_.y});particle->show();particle->enable();particle->absoluteEnable();return particle;
}
}
