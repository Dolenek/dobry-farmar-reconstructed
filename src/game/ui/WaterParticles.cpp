#include "WaterParticles.h"
#include "GameRandom.h"
#include "TextLine.h"
#include "XmlDocument.h"
#include "OriginalArithmetic.h"
#include <sstream>
#include <stdexcept>
namespace farmar::recovered {
namespace {
void number(float& value,const XmlNode& node,const wchar_t* key) {const auto text=node.attribute(WString(key));if(!text.empty()) {std::wistringstream stream(text.c_str());stream>>value;}}
void pair(Point2& value,const XmlNode& node,const wchar_t* key) {const auto text=node.attribute(WString(key));if(!text.empty()) {std::wistringstream stream(text.c_str());stream>>value[0]>>value[1];}}
float particleElapsed() {if(!uiElapsedTime()) throw std::logic_error("Water particle timer is not bound");return uiElapsedTime()();}
void advance(Picture& particle,WaterParticleMotion& m,const Point2& gravity,float dt) {
    m.velocity[0]=float(originalAdd(originalMultiply(dt,gravity[0]),m.velocity[0]));
    m.velocity[1]=float(originalAdd(originalMultiply(dt,gravity[1]),m.velocity[1]));
    const auto& g=particle.geometry();const float dx=float(originalMultiply(dt,m.velocity[0]));
    particle.setPosition({float(originalAdd(dx,g.x)),float(originalAdd(originalMultiply(dt,m.velocity[1]),g.y))});
    const auto next=originalAdd(dt,m.elapsed);m.elapsed=float(next);const float ratio=float(originalDivide(next,m.lifetime));
    const float sx=float(originalMultiply(m.maximumScale,m.originalSize[0]));const float dxSize=float(originalMultiply(sx,ratio));
    particle.setSize({float(originalAdd(dxSize,m.originalSize[0])),float(originalAdd(originalMultiply(originalMultiply(m.maximumScale,m.originalSize[1]),ratio),m.originalSize[1]))});
    const auto alpha=originalSubtract(2,originalAdd(ratio,ratio));particle.setAlpha(float(alpha>1 ? 1:alpha));
    if(m.elapsed>m.lifetime) m.finished=true;
}
}
Point2& waterParticleGravity() noexcept {static Point2 gravity{0,10};return gravity;}
HoseParticle::HoseParticle(const HoseParticle& other):Picture(other),motion(other.motion),gravity(0) {motion.finished=false;motion.elapsed=0;}
void HoseParticle::loadXML(const XmlNode& node) {
    Picture::loadXML(node);number(motion.maximumScale,node,L"MaxScale");motion.originalSize={geometry_.width,geometry_.height};motion.maximumScale=float(originalSubtract(motion.maximumScale,1));
}
void HoseParticle::enabledStep() {Picture::enabledStep();}
void HoseParticle::absoluteStep() {const float dt=particleElapsed();markTransformDirty();animation_.time=float(originalAdd(dt,animation_.time));advance(*this,motion,{0,gravity},dt);}
SprinkleParticle::SprinkleParticle(View* parent,ResourceManager& resources,GameRandom& random,ViewRegistry& views,ControlRegistry& controls):Picture(parent,resources,views,controls),random_(random) {}
SprinkleParticle::SprinkleParticle(const SprinkleParticle& other):Picture(other),motion(other.motion),random_(other.random_) {motion.finished=false;motion.elapsed=0;}
void SprinkleParticle::enabledStep() {Picture::enabledStep();}
void SprinkleParticle::loadXML(const XmlNode& node) {
    Picture::loadXML(node);pair(motion.velocity,node,L"Speed");Point2 life{0,0};pair(life,node,L"LiveTime");number(motion.maximumScale,node,L"MaxScale");motion.maximumScale=float(originalSubtract(motion.maximumScale,1));motion.originalSize={geometry_.width,geometry_.height};
    const auto random=random_.next();motion.lifetime=float(originalAdd(originalMultiply(originalMultiply(originalSubtract(life[1],life[0]),random),3.0518509447574615e-05f),life[0]));
    pair(waterParticleGravity(),node,L"Gravity");
}
void SprinkleParticle::absoluteStep() {const float dt=particleElapsed();markTransformDirty();animation_.time=float(originalAdd(dt,animation_.time));advance(*this,motion,waterParticleGravity(),dt);}
}
