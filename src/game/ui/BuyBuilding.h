#pragma once
#include "MovableUnion.h"
#include "PictureButton.h"
#include "ExtendedTextButton.h"
#include "PriceText.h"
#include "GameConfiguration.h"
#include "ProfileProperties.h"
#include <array>
namespace farmar::recovered {
class GameMenu;class TutorialManager;class SoundManager;class cSound;
struct BuildingPurchaseWorld {
    virtual ~BuildingPurchaseWorld()=default;
    virtual GameMenu& hud()=0;
    virtual TutorialManager& tutorial()=0;
    virtual int money()=0; // 00410ce0
    virtual void chooseBuilding(unsigned,int)=0; // Manufacture 00494390
};
struct BuyBuildingContext {
    ResourceManager& resources;Settings& settings;StringManager& strings;InputBindings& input;ButtonInteraction& interaction;
    ProfileProperties& profile;GameConfiguration& configuration;SoundManager& sound;BuildingPurchaseWorld& world;
    ViewRegistry& views;ControlRegistry& controls;
};
struct BuildingPurchaseDefinition {
    WString name;AnimableAppearance appearance; // native 68 bytes, 0048df00
    void loadXML(const XmlNode&,ResourceManager&); // 0048e030
};
class BuildingPurchaseButton final:public PictureButton {
public:using PictureButton::PictureButton;unsigned building=0; // native +280, 644 bytes
};
struct BuildingPurchaseCell {
    BuildingPurchaseButton* button=nullptr;ExtendedButton* mask=nullptr;TextMoney* price=nullptr;TextLine* name=nullptr;
    // Native stride is 20 bytes; the fifth word is never initialized or used.
};
class BuyBuilding;
class BuyBuildingLifetime {
protected:explicit BuyBuildingLifetime(BuyBuilding*);~BuyBuildingLifetime();
};
class BuyBuilding final:private BuyBuildingLifetime,public MovableUnion {
public:
    BuyBuilding(View*,BuyBuildingContext&); // 00491690, native 1700 bytes
    ~BuyBuilding() override; // 0048edf0
    void loadXML(const XmlNode&) override; // 0048eef0
    void enabledStep() override; // 0048dea0
    void open(); // 004928e0
    void close(); // 0048e130
    void cancel(); // inline sequence in 0048e9e0
    void choose(BuildingPurchaseButton&); // 0048e980
    bool cursorInside(); // 0048e550
    const std::array<BuildingPurchaseCell,11>& cells() const noexcept {return cells_;}
    std::array<Point2,11> places{};std::array<BuildingPurchaseDefinition,11> definitions;
    cSound* buySound() const noexcept {return buySound_;}
    static BuyBuilding* instance() noexcept;
    static constexpr std::array<std::array<unsigned,2>,11> tooltipProducts={{{3,9},{7,8},{0,10},{1,11},{13,12},{10,13},{9,14},{8,15},{14,16},{17,18},{7,17}}}; // 0065d298
private:
    BuyBuildingContext& context_;std::array<BuildingPurchaseCell,11> cells_{};cSound* buySound_=nullptr;
    static void chosen(ButtonPrototype*);static void outsidePressed(void*);
};
}
