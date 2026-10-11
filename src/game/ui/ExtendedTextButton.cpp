#include "ExtendedTextButton.h"
#include "MultilineText.h"
#include <stdexcept>
namespace farmar::recovered {
namespace {
template<class T> void attribute(const XmlNode& node,const wchar_t* key,T& value) {
    const auto text=node.attribute(WString(key));if(!text.empty()) {std::wistringstream stream(text.c_str());stream>>value;}
}
}
ExtendedTextAnimationButton::ExtendedTextAnimationButton(View* parent,ResourceManager& resources,Settings& settings,
    StringManager& strings,InputBindings& input,ButtonInteraction& interaction,std::function<bool()> tutorialActive,
    ViewRegistry& views,ControlRegistry& controls)
    :TextAnimationButton(parent,resources,settings,strings,input,interaction,views,controls),
    tutorialActive_(std::move(tutorialActive)),settings_(settings),strings_(strings),
    image_(new Picture(this,resources,views,controls)) {
    if(!tutorialActive_) throw std::invalid_argument("Extended button requires tutorial state");
}
void ExtendedTextAnimationButton::loadXML(const XmlNode& node) {
    TextAnimationButton::loadXML(node);
    XmlNode* tip=nullptr;if(node.section(String("Tip"),tip)) {
        const auto offset=tip->attribute(WString(L"Offset"));if(!offset.empty()) {std::wistringstream stream(offset.c_str());stream>>tipOffset[0]>>tipOffset[1];}
        attribute(*tip,L"TipsAlphaChangeTime",alphaChangeTime);attribute(*tip,L"WaitTime",waitTime);
        XmlNode* section=nullptr;if(tip->section(String("Image"),section)) image_->loadXML(*section);
        tips_.push_back(image_);
        const char* names[]={"UpLeftImage","UpRightImage","DownLeftImage","DownRightImage"};
        for(unsigned i=0;i<4;++i) if(tip->section(String(names[i]),section)) appearances_[i].loadXML(*section,resources_);
        if(tip->section(String("Texts"),section)) for(const auto& entry:section->children()) {
            auto* control=new TextLine(this,resources_,settings_,strings_,registry_,controls_);control->loadXML(*entry.second);tips_.push_back(control);
        }
        if(tip->section(String("Pics"),section)) for(const auto& entry:section->children()) {
            auto* control=new Picture(this,resources_,registry_,controls_);control->loadXML(*entry.second);tips_.push_back(control);
        }
        if(tip->section(String("MultiTexts"),section)) for(const auto& entry:section->children()) {
            auto* control=new MultilineText(this,resources_,settings_,strings_,registry_,controls_);control->loadXML(*entry.second);tips_.push_back(control);
        }
    }
    // Native reload appends both arrays, even another occurrence of image_.
    // Positioning still reads the earliest saved entries by index.
    for(auto* control:tips_) {
        control->setAlpha(0);control->hide();layouts_.push_back({{control->geometry().x,control->geometry().y},control->deep()});
    }
}
void ExtendedTextAnimationButton::positionTips() {
    const float x=geometry_.x,y=geometry_.y;
    unsigned index;Point2 offset=tipOffset;
    if(x<=400) {if(y<=300) index=3;else {index=1;offset[1]=-offset[1];}}
    else {offset[0]=-offset[0];if(y<=300) index=2;else {index=0;offset[1]=-offset[1];}}
    image_->copyAppearance(appearances_[index]);
    for(std::size_t i=0;i<tips_.size();++i) {
        const auto& layout=layouts_.at(i);
        tips_[i]->setPosition({static_cast<float>(originalAdd(originalAdd(offset[0],x),layout.position[0])),
            static_cast<float>(originalAdd(originalAdd(offset[1],y),layout.position[1]))});
        tips_[i]->setDeep(layout.deep);
    }
}
void ExtendedTextAnimationButton::enabledStep() {
    ButtonPrototype::enabledStep();
    if(tutorialActive_()) tipsDirection=1;
    const float elapsed=uiElapsedTime()();
    if(tipsDirection==0 && waitRemaining!=0) {
        const auto remaining=originalSubtract(waitRemaining,elapsed);waitRemaining=static_cast<float>(remaining);
        if(remaining<=0) waitRemaining=0;
    } else if(!(tipsAlpha>=1) || tipsDirection!=0) {
        if(!(tipsAlpha<=0) || tipsDirection!=1) {
            tipsAlpha=static_cast<float>(originalAdd(originalMultiply(originalDivide(tipsDirection==1 ? -1:1,alphaChangeTime),elapsed),tipsAlpha));
            for(auto* control:tips_) {
                control->setAlpha(tipsAlpha);
                if(tipsAlpha<=0) control->hide();else if(!control->state().shown || !control->state().parentShown) control->show();
            }
        }
    } else positionTips();
}
void ExtendedTextAnimationButton::onEnabled() {
    ButtonPrototype::onEnabled();for(auto* control:tips_) control->hide();tipsAlpha=0;
    if(!interaction_.cursor) throw std::logic_error("Extended button cursor service is not bound");
    if(pointInRect(interaction_.cursor())) {tipsDirection=0;positionTips();tipsAlpha=0;waitRemaining=waitTime;}
}
void ExtendedTextAnimationButton::onDisabled() {
    for(auto* control:tips_) control->hide();ButtonPrototype::onDisabled();tipsAlpha=0;tipsDirection=1;
}
void ExtendedTextAnimationButton::hovered() {
    ButtonPrototype::hovered();if(!tutorialActive_() && !tipsDisabled) {tipsDirection=0;waitRemaining=waitTime;positionTips();}
}
void ExtendedTextAnimationButton::unhovered() {
    ButtonPrototype::unhovered();if(!tutorialActive_()) {tipsDirection=1;tipsDisabled=false;}
}
void ExtendedTextAnimationButton::clicked() {ButtonPrototype::clicked();tipsDirection=1;}
void ExtendedTextAnimationButton::unclicked() {ButtonPrototype::unclicked();tipsDirection=1;}
void ExtendedTextAnimationButton::hideTips() {hide();for(auto* control:tips_) control->hide();}
}
