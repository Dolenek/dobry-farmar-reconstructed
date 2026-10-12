#pragma once
#include "MovableUnion.h"
#include "ButtonText.h"
#include "PriceText.h"
#include "MultilineText.h"
#include "GameConfiguration.h"
#include "ProfileProperties.h"
#include <array>
namespace farmar::recovered {
class GameMenu;class TutorialManager;class SoundManager;class cSound;
struct SeedPurchaseWorld {
    virtual ~SeedPurchaseWorld()=default;
    virtual GameMenu& hud()=0; // 0065f740
    virtual TutorialManager& tutorial()=0; // 0065f73c
    virtual int money()=0; // 00410ce0, get-or-insert profile Money
    virtual void chooseSeed(unsigned)=0; // actual Farm 0042c960
};
struct BuySeedsContext {
    ResourceManager& resources;Settings& settings;StringManager& strings;InputBindings& input;ButtonInteraction& interaction;
    ProfileProperties& profile;GameConfiguration& configuration;SoundManager& sound;SeedPurchaseWorld& world;
    ViewRegistry& views;ControlRegistry& controls;
};
struct SeedPurchaseDefinition {
    WString name;AnimableAppearance appearance; // native 68 bytes
    void loadXML(const XmlNode&,ResourceManager&); // 0041da60
};
class SeedPurchaseButton final:public AnimationButton {
public:using AnimationButton::AnimationButton;unsigned seed=0; // native +27c, assigned by opening
};
struct SeedPurchaseCell {SeedPurchaseButton* button=nullptr;TextMoney* price=nullptr;MultilineText* name=nullptr;};
class BuySeeds;
class BuySeedsLifetime {
protected:explicit BuySeedsLifetime(BuySeeds*);~BuySeedsLifetime();
};
class BuySeeds final:private BuySeedsLifetime,public MovableUnion {
public:
    BuySeeds(View*,BuySeedsContext&); // 00420cb0, native 1332 bytes
    ~BuySeeds() override; // 004200f0
    void loadXML(const XmlNode&) override; // 004201f0
    void enabledStep() override; // 0041d520
    void open(); // 00420f50
    void close(); // 0041d580, no HUD restoration
    void cancel(); // 0041d5e0
    void choose(SeedPurchaseButton&); // 0041d610
    bool cursorInside(); // 0041e050, includes original sound miscast read
    TextAnimationButton& cancelButton() noexcept {return *cancel_;}
    const std::array<SeedPurchaseCell,7>& cells() const noexcept {return cells_;}
    std::array<Point2,7> places{};std::array<SeedPurchaseDefinition,8> definitions;
    cSound* buySound() const noexcept {return buySound_;}
    static BuySeeds* instance() noexcept;
private:
    BuySeedsContext& context_;TextAnimationButton* cancel_;std::array<SeedPurchaseCell,7> cells_{};cSound* buySound_=nullptr;
    static void cancelled(ButtonPrototype*);static void chosen(ButtonPrototype*);static void outsidePressed(void*);
};
}
