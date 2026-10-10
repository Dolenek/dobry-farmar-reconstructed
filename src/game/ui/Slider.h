#pragma once
#include "Button.h"
namespace farmar::recovered {
class Slider;
class SliderThumb final : public Button {
public:
    SliderThumb(Slider& owner,ResourceManager& resources,InputBindings& input,ButtonInteraction& interaction,ViewRegistry& views,ControlRegistry& controls);
    void clicked() override; // 005021a0
    void unclicked() override; // 00502590
    void onHidden() override; // 005020f0
private:
    Slider& owner_;
};
// cSlider: 668 native bytes, owns a texture-switching 636-byte Button thumb.
class Slider final : public Button {
public:
    using ValueChanged=void(*)(Slider*,float);
    using DragChanged=void(*)(Slider*,bool);
    Slider(View* parent,ResourceManager& resources,InputBindings& input,ButtonInteraction& interaction,
        ViewRegistry& views=globalViewRegistry(),ControlRegistry& controls=globalControlRegistry()); // 00502970
    void loadXML(const XmlNode& node) override; // 00503bf0
    void enabledStep() override; // 005036d0, deliberately omits base step
    void clicked() override; // 00502dc0
    void unclicked() override; // 00502170
    void updateThumb(); // 005031a0
    float value() const noexcept { return value_; }
    void setValue(float value) noexcept { value_=value; } // original mutable float accessor
    bool dragging() const noexcept { return dragging_; }
    unsigned layout=0;
    float maximum=0,minimum=0;
    ValueChanged valueChanged=nullptr;
    DragChanged dragChanged=nullptr;
    SliderThumb& thumb() noexcept { return *thumb_; }
private:
    friend class SliderThumb;
    SliderThumb* thumb_;
    bool dragging_=false;
    float value_=0;
    void setDragging(bool value);
};
}
