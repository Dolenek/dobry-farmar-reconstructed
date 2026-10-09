#pragma once
#include "Control.h"
#include "InputBindings.h"
#include "ClickMask.h"
namespace farmar::recovered {
class cResource;
class ButtonPrototype;
struct ButtonInteraction {
    ButtonPrototype* active=nullptr; unsigned pending=0; // 006604dc / 006604e0
    std::function<Point2()> cursor; // cursor singleton virtual +10
    std::function<cResource*(const String&)> loadSound;
    std::function<void(cResource*,bool,unsigned)> playHover; // sound manager virtual +4 (sound,0,1)
    std::function<void(cResource*)> playClick; // sound manager virtual +8
};
enum class ButtonState:unsigned { Disabled=0,Normal=1,Hovered=2,Pressed=3,Uninitialized=0xffffffffU };
class ButtonPrototype : public Picture {
public:
    using Event=void(*)(ButtonPrototype*);
    ButtonPrototype(View* parent,ResourceManager& resources,InputBindings& input,ButtonInteraction& interaction,
        ViewRegistry& views=globalViewRegistry(),ControlRegistry& controls=globalControlRegistry()); // 004f53b0
    ~ButtonPrototype() override; // 004f5480
    void enabledStep() override; // 004f28c0
    void onEnabled() override; // 004f2770
    void onDisabled() override; // 004f2cc0
    void loadXML(const XmlNode& node) override; // 004f4b80
    virtual void setState(ButtonState state); // 004f30e0
    virtual void onStateChanged(ButtonState) {} // original empty slot 00436470
    virtual bool pointInRect(const Point2& point) const; // 004f34c0
    virtual void clicked(); // 004f41f0
    virtual void unclicked(); // 004f4620
    virtual void hovered(); // 004f3a00
    virtual void unhovered(); // 004f3e00
    ButtonState buttonState() const noexcept { return buttonState_; }
    bool deepProcessing=false,soundEnabled=true,soundOnUnclick=false;
    Event clickEvent=nullptr,hoverEvent=nullptr,unhoverEvent=nullptr,pressEvent=nullptr,unusedEvent=nullptr;
    using MaskRow=ClickMaskColumn;
    void setMask(std::vector<MaskRow> rows) { mask_=std::move(rows); maskEnabled_=true; }
protected:
    ButtonState buttonState_=ButtonState::Uninitialized;
    InputBindings& input_; ButtonInteraction& interaction_;
    cResource* clickSound_=nullptr; cResource* hoverSound_=nullptr;
private:
    bool maskEnabled_=false;
    std::vector<MaskRow> mask_;
    static void pressedInput(void* view); // 004f2620
    static void releasedInput(void* view); // 004f2690
    static void mouseInput(unsigned axis,int delta,void* view); // 004f26b0
    Point2 cursor() const;
    void playClickSound();
};
class AnimationButton : public ButtonPrototype {
public:
    using ButtonPrototype::ButtonPrototype; // 004f1c30
    void onStateChanged(ButtonState state) override; // 004f1ce0
    void loadXML(const XmlNode& node) override; // 004f20f0
    unsigned normalFrame=0,pressedFrame=0,hoverFrame=0,disabledFrame=0;
};
class Button : public ButtonPrototype {
public:
    using ButtonPrototype::ButtonPrototype; // 00507ec0
    ~Button() override; // 00508120
    void onStateChanged(ButtonState state) override; // 00507fd0
    void loadXML(const XmlNode& node) override; // 00508560
private:
    std::array<cTexture*,4> stateTextures_{}; // disabled/normal/hover/pressed
};
}
