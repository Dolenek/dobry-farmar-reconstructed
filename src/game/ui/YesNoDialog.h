#pragma once
#include "ButtonText.h"
#include "MultilineText.h"
namespace farmar::recovered {
// cYesNo (native 592 bytes). The argument is the temporarily disabled view,
// not a View parent; the dialog is a root with owned controls.
class YesNoDialog final : public Picture {
public:
    using Event=void(*)(YesNoDialog*);
    YesNoDialog(View* suspended,ResourceManager& resources,Settings& settings,StringManager& strings,
        InputBindings& input,ButtonInteraction& interaction,ViewRegistry& views=globalViewRegistry(),ControlRegistry& controls=globalControlRegistry()); // 00501b60
    ~YesNoDialog() override; // 00501970
    void loadXML(const XmlNode& node) override; // 00500df0
    void onEnabled() override; // 005014d0
    void onDisabled() override; // 00501530
    void enabledStep() override; // 00501590
    void setYesEvent(Event event) noexcept { yesEvent_=event; } // 00501430
    void setNoEvent(Event event) noexcept { noEvent_=event; } // 00501480
    TextAnimationButton& yesButton() noexcept { return *yes_; }
    TextAnimationButton& noButton() noexcept { return *no_; }
    MultilineText& text() noexcept { return *text_; }
    View* suspendedView() const noexcept { return suspended_; }
private:
    InputBindings& input_;View* suspended_;
    TextAnimationButton *yes_,*no_;MultilineText* text_;
    Event yesEvent_=&discard,noEvent_=&discard;
    static void yesClicked(ButtonPrototype* button); // 00500650
    static void noClicked(ButtonPrototype* button); // 00500a20
    static void yesInput(void* button);
    static void noInput(void* button);
    static void discard(YesNoDialog* self); // 00501a20
};
}
