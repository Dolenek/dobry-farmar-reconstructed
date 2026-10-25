#pragma once
#include "TextLine.h"
namespace farmar::recovered {
// cMultilineText, native 636 bytes; modern layout and owned View children.
class MultilineText final : public Control {
public:
    MultilineText(View* parent,ResourceManager& resources,Settings& settings,StringManager& strings,
        ViewRegistry& views=globalViewRegistry(),ControlRegistry& controls=globalControlRegistry()); // 004f5e70
    MultilineText(const MultilineText&); //004f6000, fresh empty text and owned prototype copy
    Point2 measuredSize(); //004f5db0
    void setPackedColor(unsigned value) {textColor_=Color::fromARGB(value);} //004f5cb0, no dirty/recolor
    void loadXML(const XmlNode& node) override; // 004f8580
    void setString(const WString& value,bool immediate=false); // 004f8ce0, /n -> LF
    void makeLines(); // 004f7e10
    void enabledStep() override; // 004f8e10
    void onShown() override { View::onShown(); } // 004f5c90
    void onHidden() override { View::onHidden(); } // 004f5ca0
    void setAlpha(float value) override; // 004f6a80
    const WString& string() const noexcept { return text_; }
    const std::vector<TextLine*>& lines() const noexcept { return lines_; }
    const TextLine& prototype() const noexcept { return *prototype_; }
    bool textDirty() const noexcept { return textDirty_; }
    bool localized() const noexcept { return localized_; }
    bool wraps() const noexcept { return wrap_; }
    unsigned verticalAlignment() const noexcept { return verticalAlignment_; }
    float interlinear() const noexcept { return interlinear_; }
    float multiplier() const noexcept {return prototype_->multiplier();} // 004f6260
    void setMultiplier(float value) {prototype_->setMultiplier(value);} // 004f6630, no multiline dirty change
    void setInterlinear(float value) {interlinear_=value;textDirty_=true;} // 004f6e80
    const Color& textColor() const noexcept { return textColor_; }
    void setLocalized(bool value) noexcept { localized_=value; } // 004f5fb0, no dirty flag
    void setWrap(bool value) noexcept { wrap_=value; } // 004f5fd0
    void setVerticalAlignment(unsigned value) noexcept { verticalAlignment_=value; } // 004f5ff0
private:
    StringManager& strings_;TextLine* prototype_;std::vector<TextLine*> lines_;WString text_;
    bool textDirty_=true,localized_=true,wrap_=false;
    float interlinear_=40;unsigned verticalAlignment_=3;Color textColor_{1,1,1,1};
    TextLine* createLine(unsigned index); // 004f7250
    void wrapLine(const WString& text,unsigned& index,TextLine* line); // 004f7700
    static WString normalize(const WString& value);
};
}
