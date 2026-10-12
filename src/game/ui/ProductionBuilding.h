#pragma once
#include "ExtendedTextButton.h"
#include "ProductionControls.h"
#include "ProductionLevel.h"
#include "ProductionKinds.h"
#include "ProductionPlacement.h"
#include "ProgressBar.h"
#include "Slider.h"
#include "Upgrade.h"
#include "ProfileProperties.h"
#include "GameConfiguration.h"
#include <array>
#include <vector>
namespace farmar::recovered {
class GameMenu;class TutorialManager;class SoundManager;class cSound;
class ProductionBuilding;
class ProductionSubstrateButton;
// Mandatory real world services used by original production-house methods.
// No fallback implementations: the Manufacture/warehouse/game assembly must
// supply them. Diagnostic controlled providers do not implement a game world.
struct ProductionBuildingWorld {
    virtual ~ProductionBuildingWorld()=default;
    virtual TutorialManager& tutorial()=0;
    virtual GameMenu& hud()=0;
    virtual ProductionBuilding*& movingBuilding()=0; // 00660150
    virtual ProductionBuilding* focusedBuilding()=0; // Manufacture +f8
    virtual std::vector<ProductionBuilding*> buildings()=0; // 00498400, slot order
    virtual bool manipulationBlocked()=0; // 00493b70
    virtual bool validPlacement(ProductionBuilding&)=0; // 00494a20
    virtual void beginManipulation()=0; // 004957c0
    virtual void endManipulation()=0; // 00495bd0
    virtual void focus(ProductionBuilding*,bool)=0; // 00494e70
    virtual void remove(ProductionBuilding&)=0; // 00498c60, original discard semantics
    virtual bool scenesMoving()=0; // original three StateView movement fields
    virtual unsigned warehouseGoods(unsigned)=0; // 00467fa0
    virtual unsigned warehouseFree()=0; // 00465e70
    virtual void giveWarehouse(unsigned,unsigned,bool,bool)=0; // 00467ec0
    virtual void takeWarehouse(unsigned,unsigned,bool,bool)=0; // 004682d0
    virtual int money()=0; // 00410ce0
    virtual void setMoney(int)=0; // 00410d60, includes real HUD notifications
};
struct ProductionBuildingContext {
    ResourceManager& resources;Settings& settings;StringManager& strings;InputBindings& input;ButtonInteraction& interaction;
    ProfileProperties& profile;GameConfiguration& configuration;SoundManager& sound;UpgradeContext& upgrades;
    ProductionBuildingWorld& world;ViewRegistry& views;ControlRegistry& controls;
};
// Shared controller of the 1588-byte native production houses, 0049e9f0.
// Manufacture/world assembly and runtime comparison are still in progress.
class ProductionBuilding final:public ExtendedPictureButton,public Upgrade {
public:
    ProductionBuilding(unsigned type,unsigned slot,View*,ProductionBuildingContext&);
    ~ProductionBuilding() override; // 0049b080
    void loadXML(const XmlNode&) override; // 004a0330
    void absoluteStep() override; // 004a1c90
    void onEnabled() override; // 0049dc00
    void onDisabled() override; // 0049c490
    void hovered() override; // 0049d340
    void unhovered() override; // 0049d7b0
    void clicked() override; // 0049f8c0
    void unclicked() override; // 0049fc30
    void refresh() override; // Upgrade +4, 004a2490
    void loadLevel(unsigned,const XmlNode&) override; // Upgrade +8, 0049cd60
    unsigned productionCount(); // 0049f010, uints ProdCount{propertyId}
    float productionProgress(); // 0049f0c0, floats ProdPct{propertyId}
    bool productionStopped(); // 0049f170, nonzero uints ProdStop{propertyId}
    void setProductionCount(unsigned); // 0049fdf0; decompiler float signature is incorrect
    void setProductionProgress(float); // 0049f300
    void setProductionStopped(bool); // 0049f210
    void positionProduction(const Point2&,bool save); // 0049f3c0
    void cancelMove(); // 0049f7d0, restore saved press position, hide placement pictures
    void showControls();void hideControls(); // 00499f00 / 00499c00
    bool refreshHover();bool cursorInControls(); // 00499fe0 / 0049a0b0
    ButtonPrototype& takeIcon() noexcept {return *take_;}ButtonPrototype& givenIcon() noexcept {return *given_;}
    ProductionStartStopButton& startStopButton() noexcept {return *startStop_;}
    TextLine& warehouseQuantity() noexcept {return *warehouseQty_;}TextLine& loadedQuantity() noexcept {return *loadedQty_;}
    ProgressBar& loadedProgress() noexcept {return *loadedFloat_;}Slider& loadedSlider() noexcept {return *loaded_;}
    AnimationButton& addStockButton() noexcept {return *addStock_;}AnimationButton& addDepotButton() noexcept {return *addDepot_;}
    Picture& loadedSubstrate() noexcept {return *loadedSubstrate_;}Picture& inverseSubstrate() noexcept {return *inverseSubstrate_;}
    Button& sensorPanel() noexcept {return *sensorPanel_;}Picture& lighter() noexcept {return *lighter_;}
    const std::array<cSound*,5>& sounds() const noexcept {return sounds_;}
    std::array<ProductionLevel,4> levels; // no invented numeric initialization
    unsigned buildingType,flags=0; // native +390/+574
    bool built=false;float originalDepth,productionSpeed=0; // depth assigned by XML, native ctor leaves +57c unspecified
    Point2 pressCursor{},dragOffset{},pressPosition{};
    Color normalColor{},unfitColor{},fitSubstrate{},unfitSubstrate{};
    unsigned iconType; // native +61c, read by XML; constructor leaves unspecified
private:
    friend class ProductionSubstrateButton;
    ProductionBuildingContext& context_;
    ProductionBuilding** movingAddress_; // stable 00660150 address; teardown must not dispatch through destroyed scene
    ButtonPrototype *take_,*given_;ProductionStartStopButton* startStop_;
    TextLine *warehouseQty_,*loadedQty_;ProgressBar* loadedFloat_;Slider* loaded_;
    AnimationButton *addStock_,*addDepot_;Button* sensorPanel_;
    Picture *loadedSubstrate_,*inverseSubstrate_,*lighter_;
    std::vector<ButtonPrototype*> uiControls_;
    std::array<cSound*,5> sounds_{}; // jobDone, build, take, drop, destroy
    unsigned capacity(); // includes original level -1 tooltip storage overlap
    static void toggleStop(ButtonPrototype*);static void addStock(ButtonPrototype*);static void returnStock(ButtonPrototype*);
    static void loadedValue(Slider*,float);static void loadedDrag(Slider*,bool);
};
}
