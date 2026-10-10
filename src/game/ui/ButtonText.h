#pragma once
#include "Button.h"
#include "TextLine.h"
#include "XmlDocument.h"
#include "OriginalArithmetic.h"
#include <sstream>
namespace farmar::recovered {
class ButtonText : public TextLine {
public:
    using TextLine::TextLine; // native cBtnText inside both TextButton specializations
    ButtonText(const ButtonText& source):TextLine(source),offset(source.offset),speed(source.speed) {} // 0051d8a0, motion flags reset
    void enabledStep() override; // 0051e040 / 0041fa40
    void loadXML(const XmlNode& node) override; // 0051dbf0 / 0041f5f0
    void setHovered(bool value) noexcept { hovered_=value; moving_=true; }
    bool hovered() const noexcept { return hovered_; }
    bool moving() const noexcept { return moving_; }
    float offset=100,speed=200;
private:
    bool hovered_=false,moving_=false;
};
template<class Base> class BasicTextButton : public Base {
public:
    BasicTextButton(View* parent,ResourceManager& resources,Settings& settings,StringManager& strings,
        InputBindings& input,ButtonInteraction& interaction,ViewRegistry& views=globalViewRegistry(),ControlRegistry& controls=globalControlRegistry())
        : Base(parent,resources,input,interaction,views,controls),text_(new ButtonText(this,resources,settings,strings,views,controls)) {} // 0051fe90 / 0041e8d0
    BasicTextButton(const BasicTextButton& source):Base(source),text_(new ButtonText(*source.text_)) { text_->setParent(this); } // 0051f100 for TextButton
    void setState(ButtonState state) override {
        ButtonPrototype::setState(state);
        switch(state) { case ButtonState::Disabled:case ButtonState::Normal:case ButtonState::Pressed:text_->setHovered(false);break;
        case ButtonState::Hovered:text_->setHovered(true);break;default:break; }
    } // 0051f5a0 / 0041ed10, repeated state still starts text motion
    void setAlpha(float value) override {
        Control::setAlpha(value); text_->setAlpha(value);
    } // shared virtual setter 0051f570, including unchanged parent alpha
    void loadXML(const XmlNode& node) override {
        Base::loadXML(node); XmlNode* textNode=nullptr; if(node.section(String("Text"),textNode)) text_->loadXML(*textNode);
        bool useTextPosition=false; const auto value=node.attribute(WString(L"UseTextPosition"));
        if(!value.empty()) { std::wistringstream stream(value.c_str());stream>>useTextPosition; }
        if(!useTextPosition) text_->setPosition({this->geometry_.x,this->geometry_.y});
        text_->setDeep(static_cast<float>(originalAdd(this->deep_,1))); this->dirty_=true;
    } // 0051f9b0 / 0041f120
    void setText(const WString& value) { if(text_->string().view()!=value.view()) text_->setString(value); } // 005202d0 / 004fe470
    ButtonText& text() noexcept { return *text_; }
    const ButtonText& text() const noexcept { return *text_; }
private:
    ButtonText* text_; // owned and deleted by View, as original
};
using TextButton=BasicTextButton<Button>;
using TextAnimationButton=BasicTextButton<AnimationButton>;
}
