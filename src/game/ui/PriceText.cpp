#include "PriceText.h"
#include "XmlDocument.h"
#include "OriginalArithmetic.h"
#include <sstream>
#include <stdexcept>
namespace farmar::recovered {
void PriceText::loadXML(const XmlNode& node) {
    TextLine::loadXML(node);const auto value=node.attribute(WString(L"DegreeOffset"));
    if(!value.empty()) {std::wistringstream stream(value.c_str());stream>>degreeOffset;}
}
void PriceText::makeLetters() {
    if(!dirty_ || textureName_.empty() || fontName_.empty()) return;
    TextLine::makeLetters();unsigned run=0,index=0;
    // Native list traversal goes backward while the raw source string index
    // goes forward. It does not use localized/RTL text or glyph indices here.
    for(auto letter=letters_.rbegin();letter!=letters_.rend();++letter,++index) {
        const auto character=text_.data()[index];
        if(character<L'0' || character>L'9') {run=0;continue;}
        const auto& position=(*letter)->geometry();
        const float x=float(originalAdd(originalMultiply(originalMultiply(run%3,degreeOffset),0.3333333432674408f),position.x));
        (*letter)->setPosition({x,position.y});++run;
    }
}
TextMoney::TextMoney(View* parent,ResourceManager& resources,Settings& settings,StringManager& strings,ViewRegistry& views,ControlRegistry& controls)
    :PriceText(parent,resources,settings,strings,views,controls) {coin_=new Picture(this,resources,views,controls);}
void TextMoney::loadXML(const XmlNode& node) {
    PriceText::loadXML(node);XmlNode* coin=nullptr;
    if(!node.section(String("Coin"),coin)) throw std::runtime_error("Can't find Coin");
    coin_->loadXML(*coin);const auto& position=coin_->geometry();
    coin_->setPosition({float(originalAdd(position.x,geometry_.x)),float(originalAdd(position.y,geometry_.y))});
}
void TextMoney::onEnabled() {View::onEnabled();coin_->setFrame(0);}
void TextMoney::onDisabled() {TextLine::onDisabled();coin_->setFrame(1);}
}
