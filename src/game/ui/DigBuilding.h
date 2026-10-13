#pragma once
#include "ActionArea.h"
#include "Upgrade.h"
#include "PriceText.h"
#include "AnimableAppearance.h"
namespace farmar::recovered {
class GameConfiguration;class StateView;class DigBuilding;
struct DigBuildingWorld:ActionAreaWorld {
    virtual StateView& farmScene()=0;
    virtual bool cursorOverAreaType(unsigned,const ActionArea* excluded)=0;
    virtual int heldType()=0; // Person 0044ec00
    virtual int money()=0;virtual void setMoney(int)=0;
    virtual Point2 characterPosition()=0;
    virtual void animateHeldDigUpgrade(const Point2& from,const Point2& to,int level,bool)=0; // 0042b7a0
    virtual void refreshHeldAppearance()=0; // Person 00450f90
};
struct DigBuildingContext {ActionAreaContext area;UpgradeContext& upgrades;GameConfiguration& configuration;DigBuildingWorld& world;};
struct DigBuildingLevel {
    float digTime; // original constructor leaves this unspecified
    AnimableAppearance appearance,hovered;String maskName;
    AnimableAppearance noInstrument,noInstrumentHovered;String noInstrumentMask;
}; // native 220 bytes, source layout differs
class DigBuildingLifetime {protected:explicit DigBuildingLifetime(DigBuilding*);~DigBuildingLifetime();};
// Actual cDigBuilding, native 2220 bytes, singleton 0065f74c.
class DigBuilding final:public ActionArea,private DigBuildingLifetime,public Upgrade {
public:
    DigBuilding(View*,DigBuildingContext&); // 00429fe0
    ~DigBuilding() override=default; // 00426470
    void loadXML(const XmlNode&) override; // 004265e0
    void loadLevel(unsigned,const XmlNode&) override; // 00427bf0, ECX Upgrade
    void refresh() override; // 00427740, ECX Upgrade
    void refreshAppearance(); // 00426b10
    void enabledStep() override; // 00428310
    void hovered() override; // shared 00448b20
    void unhovered() override; // shared 0045f210
    void clicked() override; // 00427340
    unsigned investment(); // 00426f50
    ExtendedTextAnimationButton& upgradeButton() noexcept {return *upgrade_;}
    TextMoney& upgradePrice() noexcept {return *price_;}
    std::array<DigBuildingLevel,4> levels;
    static DigBuilding* instance() noexcept;
private:
    DigBuildingContext& digContext_;ExtendedTextAnimationButton* upgrade_;TextMoney* price_;
    static void upgradeClick(ButtonPrototype*); // 00426260
};
}
