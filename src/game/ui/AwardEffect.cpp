#include "AwardEffect.h"
#include "ResourceManager.h"
#include "XmlDocument.h"
#include "StringConversion.h"
#include "Sound.h"
#include "SoundManager.h"
#include "OriginalArithmetic.h"
#include <sstream>
#include <stdexcept>
namespace farmar::recovered {
namespace {
template<class T> void attribute(const XmlNode& node,const wchar_t* key,T& value) {
    const auto text=node.attribute(WString(key));if(!text.empty()) { std::wistringstream stream(text.c_str());stream>>value; }
}
void vectorAttribute(const XmlNode& node,const wchar_t* key,Point2& value) {
    const auto text=node.attribute(WString(key));if(!text.empty()) { std::wistringstream stream(text.c_str());stream>>value[0];stream.ignore(1);stream>>value[1]; }
}
void setVectorLength(Point2& point,float targetLength) { // 00421520, X keeps ratio register, Y reloads its float
    const float x=point[0],y=point[1];if(x==0 && y==0) return;
    float outputX,outputY,ratio;
#if defined(_MSC_VER) && defined(_M_IX86)
    __asm {
        fld y
        fld x
        fld st(1)
        fmulp st(2),st(0)
        fld st(0)
        fmul st(0),st(1)
        faddp st(2),st(0)
        fxch st(1)
        fsqrt
        fstp st(1)
        fdivr targetLength
        fst ratio
        fmul x
        fstp outputX
        fld ratio
        fmul y
        fstp outputY
    }
#else
    const auto extended=originalDivide(targetLength,originalSqrt(originalAdd(originalMultiply(x,x),originalMultiply(y,y))));
    ratio=static_cast<float>(extended);outputX=static_cast<float>(originalMultiply(extended,x));outputY=static_cast<float>(originalMultiply(ratio,y));
#endif
    point={outputX,outputY};
}
}
AwardEffect::AwardEffect(View* parent,unsigned index,ResourceManager& resources,SoundManager& sound,ViewRegistry& views,ControlRegistry& controls)
    :Picture(parent,resources,views,controls),index_(index),soundManager_(sound) {
    const auto xml=resources.loadSettingsXML(String("UI/Awards.xml"));loadXML(*xml);flags_|=0x20;setSize({0,0});
}
AwardEffect::~AwardEffect() { hide();disable();if(sound_) sound_->releaseReference(); }
void AwardEffect::loadXML(const XmlNode& node) {
    XmlNode *awards=nullptr,*award=nullptr,*child=nullptr;
    if(node.section(String("Awards"),awards) && awards->section(index_,award)) {
        if(award->section(String("PanelText"),child)) effect_.panelText=child->text();
        if(award->section(String("Congratulation"),child)) effect_.congratulation=child->text();
        if(award->section(String("Effect"),child)) {
            Picture::loadXML(*child);effect_.initialPosition={geometry_.x,geometry_.y};
            vectorAttribute(*child,L"PositionPanel",effect_.panelPosition);attribute(*child,L"MoveSpeed",effect_.moveSpeed);
            attribute(*child,L"AppearScaleSpeed",effect_.appearSpeed);attribute(*child,L"HoveringScale",effect_.hoverScale);attribute(*child,L"HoveringScaleSpeed",effect_.hoverSpeed);
        }
        if(award->section(String("Button"),child)) vectorAttribute(*child,L"size",effect_.buttonSize);
    }
    const auto name=node.attribute(WString(L"Sound"));
    // Native XML replacement retains any earlier sound reference; destructor
    // releases only the current pointer. Preserve, do not silently fix that path.
    if(!name.empty()) sound_=resources_.getSound(toNarrow(name));
    effect_.originalDeep=deep_;effect_.effectSize={geometry_.width,geometry_.height};
}
void AwardEffect::setPhase(unsigned value) {
    effect_.phase=value;
    if(value==5) { effect_.scale=1;setTime(0); }
    else if(value!=8 && value!=6) setTime(0);
}
void AwardEffect::advance(float elapsed) { dirty_=true;animation_.time=static_cast<float>(originalAdd(elapsed,animation_.time)); }
void AwardEffect::hoverSize() {
    setDeep(static_cast<float>(originalAdd(effect_.originalDeep,effect_.scale)));
    setSize({static_cast<float>(originalMultiply(effect_.scale,effect_.buttonSize[0])),static_cast<float>(originalMultiply(effect_.scale,effect_.buttonSize[1]))});
}
void AwardEffect::enabledStep() {
    Picture::enabledStep();
    if(!uiElapsedTime()) throw std::logic_error("AwardEffect timer is not bound");
    const float elapsed=uiElapsedTime()();
    switch(effect_.phase) {
    case 0: {
        const auto scale=originalAdd(originalMultiply(elapsed,effect_.appearSpeed),effect_.scale);effect_.scale=static_cast<float>(scale);
        if(scale>=1) { effect_.scale=1;effect_.phase=2;flags_&=~0x20U;if(sound_) soundManager_.clickSound(sound_); }
        setSize({static_cast<float>(originalMultiply(effect_.scale,effect_.effectSize[0])),static_cast<float>(originalMultiply(effect_.scale,effect_.effectSize[1]))});
        const float x=static_cast<float>(originalMultiply(originalSubtract(effect_.panelPosition[0],effect_.initialPosition[0]),effect_.scale));
        const auto y=originalMultiply(originalSubtract(effect_.panelPosition[1],effect_.initialPosition[1]),effect_.scale);
        setPosition({static_cast<float>(originalAdd(x,effect_.initialPosition[0])),static_cast<float>(originalAdd(y,effect_.initialPosition[1]))});return;
    }
    case 2:
        if(animation_.time>=animation_.length) { setTime(0);effect_.phase=1; }else advance(elapsed);return;
    case 4: {
        Point2 direction{static_cast<float>(originalSubtract(effect_.destination[0],geometry_.x)),static_cast<float>(originalSubtract(effect_.destination[1],geometry_.y))};
        setVectorLength(direction,static_cast<float>(originalMultiply(elapsed,effect_.moveSpeed)));
        Point2 position{static_cast<float>(originalAdd(direction[0],geometry_.x)),static_cast<float>(originalAdd(direction[1],geometry_.y))};
        const auto dot=originalAdd(originalMultiply(originalSubtract(effect_.destination[0],position[0]),direction[0]),originalMultiply(direction[1],originalSubtract(effect_.destination[1],position[1])));
        if(dot<=0) { position=effect_.destination;effect_.phase=5; }
        setPosition(position);flags_|=0x20;
        const auto x=originalSubtract(effect_.destination[0],position[0]),y=originalSubtract(effect_.destination[1],position[1]);
        const float totalX=static_cast<float>(originalSubtract(effect_.destination[0],effect_.panelPosition[0])),totalY=static_cast<float>(originalSubtract(effect_.destination[1],effect_.panelPosition[1]));
        const auto ratio=originalDivide(originalSqrt(originalAdd(originalMultiply(x,x),originalMultiply(y,y))),originalSqrt(originalAdd(originalMultiply(totalY,totalY),originalMultiply(totalX,totalX))));
        const auto width=originalMultiply(originalSubtract(effect_.effectSize[0],effect_.buttonSize[0]),ratio);
        const float heightDifference=static_cast<float>(originalSubtract(effect_.effectSize[1],effect_.buttonSize[1]));
        const float height=static_cast<float>(originalMultiply(heightDifference,ratio));
        setSize({static_cast<float>(originalAdd(width,effect_.buttonSize[0])),static_cast<float>(originalAdd(height,effect_.buttonSize[1]))});return;
    }
    case 6: {
        animation_.loop=true;if(effect_.hoverAnimation) advance(elapsed);
        const auto scale=originalAdd(originalMultiply(elapsed,effect_.hoverSpeed),effect_.scale);effect_.scale=static_cast<float>(scale);
        if(scale>=effect_.hoverScale) { effect_.hoverAnimation=true;effect_.scale=effect_.hoverScale;effect_.phase=7; }
        hoverSize();flags_|=0x20;return;
    }
    case 7:advance(elapsed);return;
    case 8: {
        const auto scale=originalSubtract(effect_.scale,originalMultiply(elapsed,effect_.hoverSpeed));effect_.scale=static_cast<float>(scale);if(scale<=1) effect_.scale=1;
        animation_.loop=false;
        if((effect_.hoverAnimation && animation_.time>=animation_.length) || animation_.time<=0) effect_.hoverAnimation=false;
        if(effect_.hoverAnimation) advance(elapsed);
        else if(effect_.scale<=1) { effect_.phase=5;flags_&=~0x20U;setTime(0); }
        hoverSize();return;
    }
    default:return;
    }
}
}
