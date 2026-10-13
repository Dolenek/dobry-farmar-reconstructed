#pragma once
#include "ActionArea.h"
#include "Upgrade.h"
#include "PriceText.h"
#include "AnimableAppearance.h"
namespace farmar::recovered {
class GameConfiguration;class StateView;class WaterSource;
struct WaterSourceWorld:ActionAreaWorld {
    virtual StateView& farmScene()=0;
    virtual bool cursorOverAreaType(unsigned,const ActionArea*)=0;
    virtual int heldType()=0;virtual void setHeldType(int)=0;
    virtual void updatePersonAppearance()=0; // 00450a30
    virtual void cancelPersonAction(const String& origin)=0; // +284c=1, +2838=0, optional current task cancel
    virtual int money()=0;virtual void setMoney(int)=0;
    virtual unsigned sprinkleCount()=0; // 0065ff98
    virtual bool seedMoveActive()=0;virtual bool sprinkleMoveActive()=0;
    virtual ButtonPrototype* createSprinkle(unsigned cost)=0; // actual Farm 00433980
    virtual Point2 characterPosition()=0;
    virtual void animateHeldUpgrade(const Point2& from,const Point2& to,int kind,bool)=0;
};
struct WaterSourceContext {ActionAreaContext area;UpgradeContext& upgrades;GameConfiguration& configuration;WaterSourceWorld& world;};
struct WaterSourceLevel {
    float capacity; // native constructor leaves this unspecified
    AnimableAppearance appearance,hovered;String maskName;
    AnimableAppearance noInstrument,noInstrumentHovered;String noInstrumentMask;
};
class WaterSourceLifetime {protected:explicit WaterSourceLifetime(WaterSource*);~WaterSourceLifetime();};
class WaterSource final:public ActionArea,private WaterSourceLifetime,public Upgrade {
public:
    WaterSource(View*,WaterSourceContext&); // 0046b330
    ~WaterSource() override=default; // 00468640
    void loadXML(const XmlNode&) override; // 00468750
    void loadLevel(unsigned,const XmlNode&) override; // 0046a560 ECX Upgrade
    void refresh() override; // 0046acb0 ECX Upgrade
    void refreshAppearance(); // 00469bd0
    void refreshSprinkle(); // 00468f60
    void enabledStep() override; // 0046a470
    void hovered() override; // 00448b20
    void unhovered() override; // 0045f210
    void clicked() override; // 0046a010
    void unclicked() override; // 004684c0
    double capacity(); // 004683a0 original x87 result
    unsigned investment(); // 00469400
    ExtendedTextAnimationButton& upgradeButton() {return *upgrade_;}
    ExtendedTextAnimationButton& sprinkleButton() {return *sprinkle_;}
    TextMoney& upgradePrice() {return *price_;}TextMoney& sprinklePrice() {return *sprinklePrice_;}
    unsigned sprinkleCost=0;std::array<WaterSourceLevel,4> levels;
    std::array<AnimableAppearance,3> sprinkleStates;
    float sprinkleDepthWhenMoving,sprinkleNormalDepth; // ctor unspecified, actual XML assigns
    static WaterSource* instance() noexcept;
private:
    WaterSourceContext& waterContext_;ExtendedTextAnimationButton* upgrade_;ExtendedTextAnimationButton* sprinkle_;TextMoney* price_;TextMoney* sprinklePrice_;
    static void upgradeClick(ButtonPrototype*);static void sprinkleClick(ButtonPrototype*);
};
}
