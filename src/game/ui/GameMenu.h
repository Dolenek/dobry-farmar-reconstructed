#pragma once
#include "MenuView.h"
#include "StateView.h"
#include "ContractStatus.h"
#include "GamePauseMenu.h"
#include <vector>
namespace farmar::recovered {
class Gameplay;
// Required actual world dependencies. No replacement gameplay or default services.
struct GameMenuWorld {
    virtual ~GameMenuWorld()=default;
    virtual StateView& farm()=0; // 0065f744
    virtual StateView& wareDetails()=0; // 0065f758
    virtual StateView& manufacture()=0; // 0065ffc8
    virtual View& sceneOverlay()=0; // 00660010
    virtual View& selectionOverlay()=0; // 0065ff64
    virtual View& warehouseTip()=0; // 0065ffc4
    virtual bool warehouseTipOpen() const=0; // +370
    virtual void showWarehouseTip()=0; // 00465050
    virtual void hideWarehouseTip()=0; // 00465460
    virtual void resetWarehouseTip()=0; // 004654b0
    virtual bool inputBlocked() const=0; // 0065f720 / 0065f721
    virtual std::int32_t money() const=0; // 00410ce0
    virtual std::int32_t warehouseValue() const=0; // 00466a40
    virtual std::int32_t wareValue() const=0; // 00469400
    virtual std::int32_t characterValue() const=0; // 00447b40
    virtual std::int32_t farmValue() const=0; // 00426f50
    virtual std::int32_t wareLevel() const=0; // 00417050
    virtual unsigned warehouseFree() const=0; // 00465e70
    virtual unsigned warehouseCapacity(std::int32_t level) const=0; // +538 + level*70
    virtual unsigned manufactureProductCount(unsigned)=0; // 0046eb70
    virtual bool manufactureProductAvailable(unsigned)=0; // 0046f370
    virtual bool mainCharacterPresent() const=0; // 0065ff5c
    virtual bool toolRunning(unsigned slot) const=0; // +28a8 .. +28b8
    virtual void resumeTool(unsigned kind)=0; // 0044a820
    virtual void stopTool(unsigned kind)=0; // five actual stop operations
    virtual GameplayEvent& savedContract(std::int32_t)=0; // 0042cde0
    virtual void stepGameplay()=0; // actual Gameplay::step, 004b5c60
    virtual void completeTutorial(unsigned)=0; // 004d0ac0
    virtual void openMessages()=0; // actual 00446d50
};
struct GameMenuContext {
    ResourceManager& resources;Settings& settings;StringManager& strings;InputBindings& input;ButtonInteraction& interaction;
    ProfileProperties& profile;GameConfiguration& configuration;SoundManager& sound;ContractStatusWorld& contracts;
    GamePauseContext& pause;GameMenuWorld& world;ViewRegistry& views;ControlRegistry& controls;
};
class GameMenu;
class GameMenuLifetime {
protected:explicit GameMenuLifetime(GameMenu*);~GameMenuLifetime();
};
struct GameMessage {WString text;bool read=false;}; // native 32-byte record
class GameMenu final:private GameMenuLifetime,public MenuView {
public:
    GameMenu(View*,GameMenuContext&); // 004c3430
    ~GameMenu() override; // 004c31f0
    void loadXML(const XmlNode&) override; // 004c1f90
    void absoluteStep() override; // 004c3920
    void onAbsoluteEnabled() override; // 004c30a0
    void onAbsoluteDisabled() override; // 004c3130
    void activateContract(GameplayEvent&); // 004c02c0
    void removeContract(ContractStatus&); // 004c06b0
    void setMessagesRead(bool); //00445780
    void assignMessages(const std::vector<GameMessage>&); // 004c4390
    void notify(const WString&,bool read); // 004c44a0
    const std::vector<GameMessage>& messages() const noexcept {return messages_;} // 004c0710
    void switchFarm(); // 004bef70
    void switchMarket(); // 004bf4d0
    void switchWare(); // 004bf9e0
    void refreshMarketAvailability(); // 004bd950
    void refreshWareAvailability(); // 004be740
    void requestPause(); // 004c0750, ignores movement guard
    void escape(); // 004c1b60, guards movement
    void showMessages(); // 004c1770
    void refreshMoney(); // money text section of 004c1f90
    void refreshWarehouse(); // 004beb40
    void stockChanged(bool preview); // 004bd9d0
    void suspendButtons(); // 004bdf00
    void resumeButtons(); // 004be340
    void initializeScene(); // 004c2cc0
    ExtendedTextAnimationButton& mainButton() noexcept {return *main_;}
    ExtendedTextAnimationButton& marketButton() noexcept {return *market_;}
    ExtendedTextAnimationButton& farmButton() noexcept {return *farm_;}
    ExtendedTextAnimationButton& wareButton() noexcept {return *ware_;}
    ExtendedAnimationButton& journalButton() noexcept {return *journal_;}
    Button& warehouseButton() noexcept {return *warehouse_;}
    Picture& warehouseLighter() noexcept {return *lighter_;}
    ProgressBar& warehouseProgress() noexcept {return *progress_;}
    TextLine& moneyText() noexcept {return *money_;}
    ContractStatus& contract(unsigned n) {return *status_.at(n);}
    GamePauseMenu& pauseMenu() noexcept {return *pause_;}
    unsigned contractCount=0;
    bool gameplayPaused=false,hasUnread=false;
    Color blinkColor{},normalColor{};float blinkPeriod=0,blinkRemaining=0,lighterRemaining=0;
    std::array<bool,5> resumeTools{};
    std::array<bool,3> resumeSceneButtons{}; // native +50..+52 unspecified until suspension
    static GameMenu* instance() noexcept;
private:
    GameMenuContext& context_;
    ExtendedTextAnimationButton *main_,*market_,*farm_,*ware_;
    Button* warehouse_;ProgressBar* progress_;Picture* lighter_;TextLine* money_;ExtendedAnimationButton* journal_;
    std::array<ContractStatus*,3> status_{};GamePauseMenu* pause_=nullptr;
    cSound *changeSound_=nullptr,*messageSound_=nullptr;
    std::vector<GameMessage> messages_;
    bool transitioning() const;bool upgradesAvailable();void finishSwitch();
    static void mainClick(ButtonPrototype*);static void marketClick(ButtonPrototype*);static void farmClick(ButtonPrototype*);static void wareClick(ButtonPrototype*);static void journalClick(ButtonPrototype*);static void escapeInput(void*);
};
}
