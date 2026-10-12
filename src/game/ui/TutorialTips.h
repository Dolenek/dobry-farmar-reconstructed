#pragma once
#include "ButtonText.h"
#include "MultilineText.h"
namespace farmar::recovered {
class TutorialCommonTip;
// The actual TutorialManager supplies these operations; no default world logic.
struct TutorialTipHost {
    virtual ~TutorialTipHost()=default;
    virtual void finishTip(TutorialCommonTip&)=0; // 004d0c30
    virtual bool takeCancelRequest()=0; // 0065f73c + c08, clear only when queried
    virtual void setGameplayPaused(bool)=0; // GameMenu + a4
};
struct TutorialTipContext {
    ResourceManager& resources;Settings& settings;StringManager& strings;InputBindings& input;ButtonInteraction& interaction;
    TutorialTipHost& host;ViewRegistry& views;ControlRegistry& controls;
};
class TutorialCommonTip:public View {
public:
    TutorialCommonTip(View*,TutorialTipContext&); // 004cdde0, original 84 bytes
    void loadXML(const XmlNode&) override; // 004cd400
    virtual void updatePlacement(); // 004cc780, native virtual +38
    void start(const Point2& offset,const WString& text); // 004cc7c0
    void setTextMultiplier(float); // 004ccc40, -1 restores XML default
    void setTextInterlinear(float); // 004cd020, -1 restores XML default
    Picture& image() noexcept {return *image_;}
    MultilineText& text() noexcept {return *text_;}
    TextAnimationButton& okButton() noexcept {return *ok_;}
    Point2 originalPosition{},offset{};float originalAlpha=0,defaultMultiplier=0,defaultInterlinear=0;
protected:
    TutorialTipContext& context_;Picture* image_;MultilineText* text_;TextAnimationButton* ok_;
    void createBaseControls();
    static void okClick(ButtonPrototype*); // 004cda00
};
class TutorialArrowTip:public TutorialCommonTip {
public:
    TutorialArrowTip(View*,TutorialTipContext&); // 004cc150, original 124 bytes
    void loadXML(const XmlNode&) override; // 004cb8f0
    void updatePlacement() override; // 004cb4d0
    void enabledStep() override; // 004cbd60
    void start(const Point2&,const Point2& arrowOffset,const WString&,Control* anchor); // 004cb7e0
    Picture& arrow() noexcept {return *arrow_;}
    Control* anchor=nullptr;Point2 arrowOffset{},arrowOrigin{},velocity{};float phase=0,speed=2;
protected:Picture* arrow_;
    void prepare(const Point2&,const Point2&,const WString&,Control*);
};
class TutorialDisabledAnchorTip final:public TutorialArrowTip {
public:
    TutorialDisabledAnchorTip(View*,TutorialTipContext&); // 004cc700, original 128 bytes
    void start(const Point2&,const Point2&,const WString&,Control*); // 004cc290
    bool anchorWasDisabled=false; // original +7c
private:static void okClick(ButtonPrototype*); // 004cc320
};
class TutorialActionTip:public TutorialArrowTip {
public:
    using Predicate=bool(*)(Control*);
    TutorialActionTip(View*,TutorialTipContext&); // 004ca6e0, original 128 bytes
    void start(const Point2&,const Point2&,const WString&,Control*,Predicate); // 004ca7d0
    void enabledStep() override; // 004ca690
    Predicate predicate=nullptr; // native +7c, unspecified until startup
};
class TutorialRepeatedActionTip final:public TutorialActionTip {
public:
    TutorialRepeatedActionTip(View*,TutorialTipContext&); // 004cac90, native allocation 136 bytes
    void start(const Point2&,const Point2&,const WString&,Control*,Predicate); // 004caf70
    void enabledStep() override; // 004cac10, original intentionally evaluates predicate twice
};
}
