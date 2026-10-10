#pragma once
#include "Button.h"
namespace farmar::recovered {
// Original cCheckBox, 648 bytes: Button with owned Tick Picture.
class CheckBox final : public Button {
public:
    using Changed=void(*)(CheckBox*,bool);
    CheckBox(View* parent,ResourceManager& resources,InputBindings& input,ButtonInteraction& interaction,
        ViewRegistry& views=globalViewRegistry(),ControlRegistry& controls=globalControlRegistry()); // 00504180
    void loadXML(const XmlNode& node) override; // 005042a0
    void enabledStep() override; // 00504780
    void unclicked() override; // 00504b80, toggles even if base release misses
    bool checked() const noexcept { return checked_; } // 00504260 returns native bool address
    void setChecked(bool value) noexcept { checked_=value; } // 00504270, visibility deferred
    Changed changed=nullptr; // 00504250
    Picture& tick() noexcept { return *tick_; }
private:
    Picture* tick_;bool checked_=false;
    void updateTick();
};
}
