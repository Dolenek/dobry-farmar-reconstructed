#pragma once
#include "ActionArea.h"
#include "Upgrade.h"
#include "PriceText.h"
#include "AnimableAppearance.h"
namespace farmar::recovered {
class GameConfiguration;class StateView;class ProfileProperties;class MowBuilding;
struct MowBuildingWorld:ActionAreaWorld {
    virtual StateView& farmScene()=0;
    virtual bool cursorOverAreaType(unsigned,const ActionArea*)=0;
    virtual int heldType()=0;virtual bool personPresent()=0;
    virtual unsigned heldProductCount()=0; // 0044ed10
    virtual void hidePersonHeldIndicator()=0; // Person +2850 View.hide
    virtual void updatePersonHeldIndicator(bool,unsigned count,unsigned capacity)=0; // 0044ee70
    virtual int money()=0;virtual void setMoney(int)=0;
    virtual Point2 characterPosition()=0;
    virtual void animateHeldUpgrade(const Point2& from,const Point2& to,int kind,bool)=0;
    virtual void refreshHeldAppearance()=0; // 00450f90
};
struct MowBuildingContext {ActionAreaContext area;UpgradeContext& upgrades;GameConfiguration& configuration;ProfileProperties& profile;MowBuildingWorld& world;};
struct MowBuildingLevel {
    unsigned productionNum;float mowTime; // original ctor leaves both unspecified
    AnimableAppearance appearance,hovered;String maskName;
    AnimableAppearance noInstrument,noInstrumentHovered;String noInstrumentMask;
}; // native224
class MowBuildingLifetime {protected:explicit MowBuildingLifetime(MowBuilding*);~MowBuildingLifetime();};
class MowBuilding final:public ActionArea,private MowBuildingLifetime,public Upgrade {
public:
    MowBuilding(View*,MowBuildingContext&); // 00449560
    ~MowBuilding() override=default; // 004473b0
    void loadXML(const XmlNode&) override; // 004474b0
    void loadLevel(unsigned,const XmlNode&) override; // 004483e0 ECX Upgrade
    void refresh() override; // 00449050 ECX Upgrade
    void refreshAppearance(); // 00448b60
    void enabledStep() override; // 00448330
    void hovered() override; // 00448b20
    void unhovered() override; // 0045f210
    void clicked() override; // 00447f30
    unsigned investment(); // 00447b40
    ExtendedTextAnimationButton& upgradeButton() {return *upgrade_;}
    TextMoney& upgradePrice() {return *price_;}Picture& heldProductSign() {return *sign_;}Picture& heldProductSubstrate() {return *substrate_;}
    std::array<MowBuildingLevel,4> levels;
    static MowBuilding* instance() noexcept;
private:
    MowBuildingContext& mowContext_;ExtendedTextAnimationButton* upgrade_;TextMoney* price_;Picture* sign_;Picture* substrate_;
    static void upgradeClick(ButtonPrototype*);
};
}
