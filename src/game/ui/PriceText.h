#pragma once
#include "TextLine.h"
namespace farmar::recovered {
class PriceText:public TextLine {
public:
    using TextLine::TextLine; // 0048bd30, native 700 bytes
    void loadXML(const XmlNode&) override; // 0048be00
    void makeLetters() override; // 0048c210
    float degreeOffset=4;
};
class TextMoney final:public PriceText {
public:
    TextMoney(View*,ResourceManager&,Settings&,StringManager&,
        ViewRegistry& views=globalViewRegistry(),ControlRegistry& controls=globalControlRegistry()); // 00415b70, native 704 bytes
    void loadXML(const XmlNode&) override; // 00415d10
    void onEnabled() override; // 004161a0
    void onDisabled() override; // 00416580
    Picture& coin() noexcept {return *coin_;}
private:
    Picture* coin_; // owned View child, native +2bc
};
}
