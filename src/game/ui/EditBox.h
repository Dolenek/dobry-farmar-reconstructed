#pragma once
#include "TextLine.h"
#include "InputBindings.h"
namespace farmar::recovered {
// cTextBox, original 732 bytes; modern layout with recovered owned caret.
class EditBox final : public TextLine {
public:
    using Confirm=bool(*)(wchar_t*);
    using Changed=void(*)(EditBox*);
    EditBox(View* parent,ResourceManager& resources,Settings& settings,StringManager& strings,InputBindings& input,
        ViewRegistry& views=globalViewRegistry(),ControlRegistry& controls=globalControlRegistry()); // 005213c0
    ~EditBox() override; // 00522c40
    void loadXML(const XmlNode& node) override; // 00521860
    void onEnabled() override; // 00521820
    void onDisabled() override; // 00521840
    void enabledStep() override; // 00521f00
    void process() override; // 00522860
    void makeLetters() override; // 00522320
    void character(wchar_t value); // 00521500
    Confirm confirm=nullptr; // 00521430
    Changed changed=nullptr; // 00521440
    std::function<unsigned()> keyboardLanguage;
    const Letter* caret() const noexcept { return caret_; }
    bool rightInput() const noexcept { return rightInput_; }
    bool caretShown() const noexcept { return caretShown_; }
    float blinkTime() const noexcept { return blinkTime_; }
    float maximumWidth() const noexcept { return maximumWidth_; }
    float blinkingPeriod() const noexcept { return blinkingPeriod_; }
    float caretAdvance() const noexcept { return caretAdvance_; }
private:
    InputBindings& input_;
    bool rightInput_=false,caretShown_=true;
    float maximumWidth_=10,blinkingPeriod_=0.7f,blinkTime_=0,caretAdvance_=0;
    Letter* caret_=nullptr; // View owns this separate child, not letter pool
    static void dispatch(wchar_t value,void* self);
};
}
