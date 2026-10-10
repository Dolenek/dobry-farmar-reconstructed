#include "Button.h"
#include "ResourceManager.h"
#include "Resource.h"
#include "Texture.h"
#include "XmlDocument.h"
#include "StringConversion.h"
#include "OriginalArithmetic.h"
#include <sstream>
#include <stdexcept>
namespace farmar::recovered {
namespace {
template<class T> void attribute(const XmlNode& node,const wchar_t* key,T& output) {
    const auto text=node.attribute(WString(key)); if(!text.empty()) { std::wistringstream stream(text.c_str()); stream>>output; }
}
}
ButtonPrototype::ButtonPrototype(View* parent,ResourceManager& resources,InputBindings& input,ButtonInteraction& interaction,ViewRegistry& views,ControlRegistry& controls)
    : Picture(parent,resources,views,controls),input_(input),interaction_(interaction) {}
ButtonPrototype::ButtonPrototype(const ButtonPrototype& source):Picture(source),deepProcessing(source.deepProcessing),soundOnUnclick(source.soundOnUnclick),clickEvent(source.clickEvent),hoverEvent(source.hoverEvent),unhoverEvent(source.unhoverEvent),pressEvent(source.pressEvent),unusedEvent(source.unusedEvent),buttonState_(source.buttonState_),input_(source.input_),interaction_(source.interaction_),clickSound_(source.clickSound_),hoverSound_(source.hoverSound_),maskEnabled_(source.maskEnabled_),mask_(source.mask_) {
    if(clickSound_) clickSound_->addReference();if(hoverSound_) hoverSound_->addReference();
}
ButtonPrototype::~ButtonPrototype() {
    ButtonPrototype::onDisabled();
    if(clickSound_) clickSound_->releaseReference(); if(hoverSound_) hoverSound_->releaseReference();
    if(interaction_.active==this) interaction_.active=nullptr;
}
Point2 ButtonPrototype::cursor() const { if(!interaction_.cursor) throw std::logic_error("Button cursor service is not bound"); return interaction_.cursor(); }
void ButtonPrototype::enabledStep() {
    Picture::enabledStep();
    if(deepProcessing && interaction_.active==this) {
        if(interaction_.pending==1) hovered(); else if(interaction_.pending==2) clicked();
        interaction_.pending=0;
    }
}
void ButtonPrototype::onEnabled() {
    View::onEnabled(); input_.bind(0x208,&pressedInput,this); input_.bind(0x200,&releasedInput,this); input_.bindMouse(&mouseInput,this);
    setState(pointInRect(cursor()) ? ButtonState::Hovered:ButtonState::Normal);
}
void ButtonPrototype::onDisabled() {
    View::onDisabled(); if(deepProcessing && interaction_.active==this) interaction_.active=nullptr;
    input_.unbind(0x208,&pressedInput,this); input_.unbind(0x200,&releasedInput,this); input_.unbindMouse(&mouseInput,this);
    setState(ButtonState::Disabled);
}
void ButtonPrototype::setState(ButtonState state) { if(buttonState_!=state) { onStateChanged(state); buttonState_=state; } }
bool ButtonPrototype::pointInRect(const Point2& point) const {
    if(!hitTest(point)) return false; if(!maskEnabled_) return true;
    if(mask_.empty()) throw std::logic_error("Button mask has no rows");
    // Original indexes rows by X and bits by Y. It clamps only negative values,
    // truncates with _ftol2, and uses (count-1)/control extent.
    const auto left=static_cast<float>(originalSubtract(geometry_.x,originalMultiply(geometry_.width,0.5)));
    const auto x=static_cast<float>(originalSubtract(point[0],left));
    const auto y=originalSubtract(point[1],originalSubtract(geometry_.y,originalMultiply(geometry_.height,0.5)));
    const auto rowCoordinate=originalMultiply(originalMultiply(originalUnsigned(static_cast<unsigned>(mask_.size())-1),originalDivide(1,geometry_.width)),x);
    const auto bitCoordinate=originalMultiply(originalMultiply(originalUnsigned(mask_.front().bitCount-1),originalDivide(1,geometry_.height)),y);
    const auto row=static_cast<unsigned>(rowCoordinate<0 ? 0:rowCoordinate),bit=static_cast<unsigned>(bitCoordinate<0 ? 0:bitCoordinate);
    return (mask_.at(row).words.at(bit>>5)&(1U<<(bit&31)))!=0;
}
void ButtonPrototype::hovered() {
    setState(ButtonState::Hovered); if(hoverEvent) hoverEvent(this);
    if(soundEnabled && hoverSound_ && buttonState_==ButtonState::Hovered) {
        if(!interaction_.playHover) throw std::logic_error("Button hover sound service is not bound"); interaction_.playHover(hoverSound_,false,1);
    }
}
void ButtonPrototype::unhovered() {
    if(deepProcessing && interaction_.active==this) interaction_.active=nullptr;
    setState(ButtonState::Normal); if(unhoverEvent) unhoverEvent(this);
}
void ButtonPrototype::playClickSound() { if(!interaction_.playClick) throw std::logic_error("Button click sound service is not bound"); interaction_.playClick(clickSound_); }
void ButtonPrototype::clicked() {
    if(!soundOnUnclick && soundEnabled && clickSound_) playClickSound();
    if(!state_.enabled || !state_.parentEnabled) setState(ButtonState::Disabled);
    else { setState(ButtonState::Pressed); if(unhoverEvent) unhoverEvent(this); if(pressEvent) pressEvent(this); }
}
void ButtonPrototype::unclicked() {
    if(soundOnUnclick && soundEnabled && clickSound_) playClickSound();
    if(!pointInRect(cursor())) unhovered();
    else { setState(ButtonState::Hovered); if(hoverEvent) hoverEvent(this); if(clickEvent) clickEvent(this); }
}
void ButtonPrototype::pressedInput(void* view) {
    auto& b=*static_cast<ButtonPrototype*>(view); if(b.pointInRect(b.cursor())) {
        if(!b.deepProcessing) b.clicked(); else if(b.interaction_.active==&b) b.interaction_.pending=2;
    }
}
void ButtonPrototype::releasedInput(void* view) { auto& b=*static_cast<ButtonPrototype*>(view); if(b.buttonState_==ButtonState::Pressed) b.unclicked(); }
void ButtonPrototype::mouseInput(unsigned,int,void* view) {
    auto& b=*static_cast<ButtonPrototype*>(view);
    if(!b.pointInRect(b.cursor())) {
        if(b.buttonState_==ButtonState::Hovered) b.unhovered();
        if(b.interaction_.active==&b) b.interaction_.active=nullptr;
    } else if(b.buttonState_!=ButtonState::Pressed && b.buttonState_!=ButtonState::Hovered) {
        if(!b.deepProcessing) b.hovered();
        else {
            if(b.interaction_.active) { if(b.deep_<=b.interaction_.active->deep_) return; b.interaction_.active->unhovered(); }
            b.interaction_.active=&b; b.interaction_.pending=1;
        }
    }
}
void ButtonPrototype::loadXML(const XmlNode& node) {
    Picture::loadXML(node); attribute(node,L"IsDeepProcessing",deepProcessing); attribute(node,L"SoundOnUnclick",soundOnUnclick);
    const auto mask=toNarrow(node.attribute(WString(L"texture_mask"))); maskEnabled_=!mask.empty();
    if(maskEnabled_) mask_=loadClickMask(mask);
    XmlNode* sounds=nullptr; if(node.section(String("Sounds"),sounds)) {
        const auto load=[&](const wchar_t* key,cResource*& resource) {
            const auto name=toNarrow(sounds->attribute(WString(key))); if(name.empty()) return;
            if(!interaction_.loadSound) throw std::logic_error("Button sound resource factory is not bound"); resource=interaction_.loadSound(name);
        }; load(L"Click",clickSound_); load(L"Hover",hoverSound_);
    }
}
void AnimationButton::onStateChanged(ButtonState state) {
    switch(state) { case ButtonState::Disabled:setFrame(disabledFrame);break; case ButtonState::Normal:setFrame(normalFrame);break;
    case ButtonState::Hovered:setFrame(hoverFrame);break; case ButtonState::Pressed:setFrame(pressedFrame);break; default:break; }
}
void AnimationButton::loadXML(const XmlNode& node) {
    ButtonPrototype::loadXML(node); attribute(node,L"frame_normal",normalFrame); attribute(node,L"frame_pressed",pressedFrame);
    attribute(node,L"frame_hover",hoverFrame); attribute(node,L"frame_disabled",disabledFrame); playing_=false;
    setState(state_.enabled && state_.parentEnabled ? ButtonState::Normal:ButtonState::Disabled);
}
Button::Button(const Button& source):ButtonPrototype(source),stateTextures_(source.stateTextures_) {
    for(auto* texture:stateTextures_) if(texture) texture->addReference();
}
Button::~Button() {
    // 00508120 releases normal, pressed, hovered, then disabled before the
    // prototype destructor disables the control and releases its own texture.
    for(unsigned index:{1U,3U,2U,0U}) if(stateTextures_[index]) stateTextures_[index]->releaseReference();
}
void Button::onStateChanged(ButtonState state) {
    const unsigned index=static_cast<unsigned>(state);
    if(index<4) {
        auto* replacement=stateTextures_[index];
        if(texture_) { texture_->releaseReference(); if(!replacement) replacement=texture_; }
        texture_=replacement ? static_cast<cTexture*>(replacement->addReference()):nullptr;
    }
    dirty_=true;
}
void Button::loadXML(const XmlNode& node) {
    ButtonPrototype::loadXML(node);
    const auto retain=[&] { return texture_ ? static_cast<cTexture*>(texture_->addReference()):nullptr; };
    stateTextures_[1]=retain();
    const std::pair<unsigned,const wchar_t*> keys[]={{3,L"texture_pressed"},{2,L"texture_hover"},{0,L"texture_disabled"}};
    for(const auto& key:keys) { const auto name=toNarrow(node.attribute(WString(key.second))); stateTextures_[key.first]=name.empty() ? retain():resources_.getTexture(name); }
}
}
