#pragma once
#include "StateView.h"
#include "BuyBuilding.h"
#include "ProductionBuilding.h"
#include <array>
namespace farmar::recovered {
class Manufacture;
// Required cGame/warehouse services. Manufacture implements its own factories,
// slots, manipulation and selection; these remaining world dependencies must
// be supplied by the reconstructed game, never by production default providers.
struct ManufactureWorld {
    virtual ~ManufactureWorld()=default;
    virtual GameMenu& hud()=0;
    virtual TutorialManager& tutorial()=0;
    virtual BuyBuilding& purchasePanel()=0;
    virtual StateView& farmScene()=0; // 0065f744
    virtual StateView& tradeScene()=0; // 0065ffc8
    virtual unsigned warehouseGoods(unsigned)=0;
    virtual unsigned warehouseFree()=0;
    virtual void giveWarehouse(unsigned,unsigned,bool,bool)=0;
    virtual void takeWarehouse(unsigned,unsigned,bool,bool)=0;
    virtual int money()=0;
    virtual void setMoney(int)=0;
};
struct ManufactureContext {
    ResourceManager& resources;Settings& settings;StringManager& strings;InputBindings& input;ButtonInteraction& interaction;
    ProfileProperties& profile;GameConfiguration& configuration;SoundManager& sound;UpgradeContext& upgrades;
    ManufactureWorld& world;ViewRegistry& views;ControlRegistry& controls;
};
// Ongoing translation of the actual 256-byte cManufacture, singleton 0065f758.
// Direct native comparison and whole-game assembly are still in progress.
class Manufacture final:public StateView,public ProductionBuildingWorld,public BuildingPurchaseWorld {
public:
    Manufacture(View*,ManufactureContext&); // 00499730
    ~Manufacture() override; // 00496fa0
    void loadXML(const XmlNode&) override; // 004979a0
    void enabledStep() override; // 00493dd0
    void absoluteStep() override; // 00493fd0 -> StateView
    void onEnabled() override; // 00497070
    void onDisabled() override; // 00497480
    void onShown() override; // 00497870
    ProductionBuilding* create(unsigned,int slot=-1); // 00494390
    std::vector<ProductionBuilding*> buildings() override; // 00498400
    bool manipulationBlocked() override; // 00493b70
    bool validPlacement(ProductionBuilding&) override; // 00494a20
    void beginManipulation() override; // 004957c0
    void endManipulation() override; // 00495bd0
    void focus(ProductionBuilding*,bool) override; // 00494e70
    void remove(ProductionBuilding&) override; // 00498c60
    void chooseBuilding(unsigned,int) override;
    TutorialManager& tutorial() override;
    GameMenu& hud() override;
    ProductionBuilding*& movingBuilding() override; // 00660150
    ProductionBuilding* focusedBuilding() override {return focused_;}
    bool scenesMoving() override;
    unsigned warehouseGoods(unsigned) override;
    unsigned warehouseFree() override;
    void giveWarehouse(unsigned,unsigned,bool,bool) override;
    void takeWarehouse(unsigned,unsigned,bool,bool) override;
    int money() override;void setMoney(int) override;
    ProductionRectangle allowed{};std::vector<ProductionRectangle> forbidden; // +68 / +7c
    unsigned count=0; // native +f4; equality gate, not an inferred >= capacity test
    const std::array<ProductionBuilding*,20>& slots() const noexcept {return slots_;}
    ExtendedTextAnimationButton& buildButton() noexcept {return *build_;}
    ExtendedAnimationButton& destroyButton() noexcept {return *destroy_;}ExtendedAnimationButton& upgradeButton() noexcept {return *upgrade_;}
    Picture& currentPicture() noexcept {return *currentPicture_;}TextLine& currentText() noexcept {return *currentText_;}
    TextMoney& upgradePrice() noexcept {return *price_;}
    static Manufacture* instance() noexcept;
private:
    ManufactureContext& context_;ProductionBuildingContext buildingContext_;
    ExtendedTextAnimationButton* build_;TextLine* currentText_;Picture* currentPicture_;
    ExtendedAnimationButton *destroy_,*upgrade_;TextMoney* price_;
    bool restoreUpgrade_; // native +a0 unspecified until beginManipulation
    std::array<ProductionBuilding*,20> slots_{};
    ProductionBuilding* focused_=nullptr;cSound* selectSound_=nullptr;
    static void buildClick(ButtonPrototype*);static void destroyClick(ButtonPrototype*);static void upgradeClick(ButtonPrototype*);
    static void outsidePress(void*);static void rightPress(void*);
    void refreshHouseControls(); // 004953e0
};
}
