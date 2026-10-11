#pragma once
#include "Button.h"
#include "TooltipFrame.h"
#include "AwardEffect.h"
namespace farmar::recovered {
class AwardButton final : public ButtonPrototype {
public:
    AwardButton(View* parent,unsigned index,const WString& description,ResourceManager& resources,
        Settings& settings,StringManager& strings,SoundManager& sound,InputBindings& input,ButtonInteraction& interaction,
        TooltipFrame::ScreenSize screenSize,ViewRegistry& views=globalViewRegistry(),ControlRegistry& controls=globalControlRegistry()); // 004728c0
    ~AwardButton() override; // 00472d50
    void loadXML(const XmlNode& node) override; // 00474dc0
    void setOwned(unsigned char value); // 00473150, changes appearance, not buttonState
    unsigned char owned() const noexcept { return owned_; }
    unsigned index() const noexcept { return index_; }
    void enabledStep() override; // 00474520
    void onEnabled() override; // 00473d50
    void onDisabled() override; // 00474130
    void hovered() override; // 00473520
    void unhovered() override; // 00473940
    void onStateChanged(ButtonState state) override; // 00474910
    void clicked() override {} // original virtual +150 points to empty 004bd820
    void unclicked() override {} // original virtual +154 also empty, no click events/sound
    TooltipFrame& tooltip() noexcept { return *tooltip_; }
    AwardEffect& effect() noexcept { return *effect_; }
    cTexture* normalTexture() const noexcept { return normal_; }
    cTexture* disabledTexture() const noexcept { return disabled_; }
private:
    unsigned index_;
    const WString& description_;
    cTexture *normal_=nullptr,*disabled_=nullptr;
    TooltipFrame* tooltip_;
    AwardEffect* effect_;
    unsigned char owned_=0;
};
}
