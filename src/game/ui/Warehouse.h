#pragma once
#include "ActionArea.h"
#include "Upgrade.h"
#include "PriceText.h"
#include "ProgressBar.h"
#include "AnimableAppearance.h"
namespace farmar::recovered {
class Manufacture;class GameMenu;class StateView;class ProfileProperties;class GameConfiguration;class Warehouse;
class WarehouseLifetime {
protected:explicit WarehouseLifetime(Warehouse*);~WarehouseLifetime();
};
// These are required remaining world bindings. Stock mutation, progress,
// Manufacture labels and HUD are implemented by the actual recovered classes.
// Market and WareDetail tooltip providers must implement their original effects.
struct WarehouseWorld:ActionAreaWorld {
    virtual GameMenu& hud()=0;
    virtual Manufacture& manufactureScene()=0;
    virtual StateView& farmScene()=0;
    virtual bool cursorOverAreaType(unsigned,const ActionArea* excluded)=0; // 0042d700 / 0042db30
    virtual void refreshMarketStocks()=0; // 0046f9e0, singleton 0065ffc8
    virtual void refreshWarehouseTip()=0; // 00464b40, singleton 0065ffc4
    virtual int money()=0;
    virtual void setMoney(int)=0; // 00410d60: includes other upgrade/HUD effects
};
struct WarehouseContext {
    ActionAreaContext area;UpgradeContext& upgrades;ProfileProperties& profile;
    GameConfiguration& configuration;WarehouseWorld& world;
};
// Native 112-byte level: uint capacity, two 40-byte appearances, String mask.
// 00465f40 does not initialize capacity; valid shipped XML supplies it.
struct WarehouseLevel {
    unsigned capacity;
    AnimableAppearance appearance,hovered;
    String maskName;
};
// Actual cWareHouse, native 1800 bytes, singleton 0065f748.
// Whole-world assembly and native comparison continue in WAREHOUSE.md.
class Warehouse final:public ActionArea,private WarehouseLifetime,public Upgrade {
public:
    Warehouse(View*,WarehouseContext&); // 00467fe0
    ~Warehouse() override; // 00466020
    void loadXML(const XmlNode&) override; // 00467220
    void loadLevel(unsigned,const XmlNode&) override; // 00467910, ECX Upgrade
    void refresh() override; // 00466530, ECX Upgrade
    void enabledStep() override; // 004678b0
    void hovered() override; // 00448b20
    void unhovered() override; // 0045f210 -> 00419e70
    void clicked() override; // 00466e30
    unsigned goods(unsigned product); // 00467fa0
    unsigned freeSpace(); // 00465e70, unsigned capacity - occupied
    unsigned give(unsigned product,unsigned count,bool refreshMarket,bool preview); // 00467ec0
    unsigned take(unsigned product,unsigned count,bool refreshMarket,bool preview); // 004682d0
    unsigned investment(); // 00466a40: sum indexes 1 .. level-1
    void updateLoading(); // 00466110, original x87 percent and actual ProgressBar lid
    Picture& loadingSubstrate() noexcept {return *substrate_;}
    ProgressBar& loadingProgress() noexcept {return *loading_;}
    ExtendedTextAnimationButton& upgradeButton() noexcept {return *upgrade_;}
    TextMoney& upgradePrice() noexcept {return *price_;}
    unsigned occupied=0; // native +534, accumulated after initial refresh
    std::array<WarehouseLevel,4> levels;
    static Warehouse* instance() noexcept;
private:
    WarehouseContext& warehouseContext_;
    Picture* substrate_;ProgressBar* loading_;
    ExtendedTextAnimationButton* upgrade_;TextMoney* price_;
    void stockChanged(bool refreshMarket,bool preview);
    static void upgradeClick(ButtonPrototype*); // 00465ee0
};
}
