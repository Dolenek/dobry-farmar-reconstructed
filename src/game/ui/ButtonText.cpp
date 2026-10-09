#include "ButtonText.h"
#include <stdexcept>
namespace farmar::recovered {
namespace {
float sign(double value) noexcept { return value>0 ? 1.0f:value<0 ? -1.0f:0.0f; }
}
void ButtonText::loadXML(const XmlNode& node) {
    TextLine::loadXML(node);
    for(const auto& entry:{std::pair<const wchar_t*,float*>{L"Offset",&offset},{L"Speed",&speed}}) {
        const auto text=node.attribute(WString(entry.first)); if(!text.empty()) { std::wistringstream stream(text.c_str());stream>>*entry.second; }
    }
}
void ButtonText::enabledStep() {
    TextLine::enabledStep(); if(!moving_) return;
    if(!uiElapsedTime()) throw std::logic_error("Button text timer is not bound");
    const float elapsed=uiElapsedTime()(); bool changed=false; int direction=1;
    for(auto* letter:letters()) {
        const double unspilledTarget=hovered_ ? originalAdd(originalMultiply(direction,offset),geometry_.y):geometry_.y;
        const float target=static_cast<float>(unspilledTarget),current=letter->geometry().y;
        if(unspilledTarget!=current) {
            changed=true; const float before=sign(originalSubtract(target,current));
            const auto movement=originalMultiply(originalMultiply(before,speed),elapsed);
            letter->setPosition({letter->geometry().x,static_cast<float>(originalAdd(movement,current))});
            if(sign(originalSubtract(target,letter->geometry().y))!=before) letter->setPosition({letter->geometry().x,target});
        }
        if(hovered_) direction=-direction;
    }
    moving_=changed;
}
template class BasicTextButton<Button>;
template class BasicTextButton<AnimationButton>;
}
