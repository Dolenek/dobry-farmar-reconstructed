#include "Game.h"
#include "GameConfiguration.h"
#include "GameEffects.h"
#include "GamePersistence.h"
#include "GameRandom.h"
#include "Gameplay.h"
#include "SalesHistory.h"
#include "Profile.h"
#include "Farm.h"
#include "GameMenu.h"
#include "TutorialManager.h"
#include "Market.h"
#include "Manufacture.h"
#include "Warehouse.h"
#include "WarehouseTip.h"
#include "FieldTip.h"
#include "ElasticAnimation.h"
#include "BuySeeds.h"
#include "ContractProvide.h"
#include "ContractDialogs.h"
#include "HelpView.h"
#include "Journal.h"
#include "GameAwardManager.h"
#include "FlyingText.h"
#include "ResourceManager.h"
#include "GameTimer.h"
#include "OriginalArithmetic.h"
#include <cstring>
#include <stdexcept>
namespace farmar::recovered {
namespace {
Game* currentGame=nullptr;
bool visible(const View& view) {return view.state().shown && view.state().parentShown;}
int signedBits(unsigned value) {int result;std::memcpy(&result,&value,4);return result;}
}
GameLifetime::GameLifetime(Game* game) {if(currentGame) throw "Object already exist.";currentGame=game;}
GameLifetime::~GameLifetime() {currentGame=nullptr;}
Game* Game::instance() noexcept {return currentGame;}
struct Game::World final:GameMenuWorld,TutorialWorld,WarehouseWorld,MarketWorld,ManufactureWorld,SeedPurchaseWorld,
    ContractStatusWorld,FarmGameServices,GameplayEventExecutor {
    Game& game;GameContext& c;ProfileProperties& profile;
    GameConfiguration configuration;SalesHistory sales;UpgradeContext upgrades;
    FlyingTextContext flyingContext;GameEffects effects;
    TutorialManagerContext tutorialContext;
    std::unique_ptr<Gameplay> gameplay;
    std::unique_ptr<TutorialManager> tips;
    std::unique_ptr<GameMenuContext> hudContext;
    std::unique_ptr<FarmContext> farmContext;
    std::unique_ptr<MarketContext> marketContext;
    ManufactureContext manufactureContext;
    WarehouseTipContext warehouseTipContext;FieldTipContext fieldTipContext;
    ActionAreaContext areaContext;
    WarehouseContext warehouseContext;std::unique_ptr<DigBuildingContext> digContext;std::unique_ptr<WaterSourceContext> waterContext;std::unique_ptr<MowBuildingContext> mowContext;
    std::unique_ptr<PersonContext> personContext;
    std::unique_ptr<BuyBuildingContext> buyBuildingContext;
    BuySeedsContext buySeedsContext;
    std::unique_ptr<JournalContext> journalContext;
    std::unique_ptr<ContractDialogContext> dialogContext;
    HelpViewContext helpContext;GameAwardContext awardContext;
    std::unique_ptr<FieldTip> fieldTip;
    std::unique_ptr<Journal> journal;
    std::unique_ptr<ContractProvide> provide;std::unique_ptr<ContractAim> aim;
    std::unique_ptr<ContractPassConfirm> confirm;std::unique_ptr<ContractFail> fail;
    std::unique_ptr<HelpView> help;std::unique_ptr<GameAwardManager> awards;
    WarehouseTip* stockTip=nullptr;GameMenu* menu=nullptr;Farm* fields=nullptr;Market* trade=nullptr;Manufacture* factories=nullptr;
    Warehouse* store=nullptr;DigBuilding* dig=nullptr;WaterSource* water=nullptr;MowBuilding* mow=nullptr;
    Person* actor=nullptr;BuyBuilding* buyBuilding=nullptr;BuySeeds* buySeeds=nullptr;
    std::unique_ptr<GamePersistence> persistence;
    bool complete=false;
    World(Game& game,GameContext& c):game(game),c(c),profile(c.profile.properties()),configuration(c.resources),sales(profile),upgrades(configuration,profile,c.sound),
        flyingContext{c.resources,c.settings,c.strings,c.views,c.controls},effects(game,flyingContext,configuration),
        tutorialContext{c.resources,c.settings,c.strings,c.input,c.interaction,profile,*this,c.views,c.controls},
        manufactureContext{c.resources,c.settings,c.strings,c.input,c.interaction,profile,configuration,c.sound,upgrades,*this,c.views,c.controls},
        warehouseTipContext{c.resources,c.settings,c.strings,configuration,c.views,c.controls},fieldTipContext{c.resources,c.settings,c.strings,configuration,c.views,c.controls},
        areaContext{c.resources,c.settings,c.strings,c.input,c.interaction,*this,c.views,c.controls},
        warehouseContext{areaContext,upgrades,profile,configuration,*this},
        buySeedsContext{c.resources,c.settings,c.strings,c.input,c.interaction,profile,configuration,c.sound,*this,c.views,c.controls},
        helpContext{c.resources,c.settings,c.strings,c.input,c.interaction,c.views,c.controls},
        awardContext{c.resources,c.settings,c.strings,c.sound,c.input,c.interaction,profile,game,*this,c.screenSize,c.views,c.controls} {}
    Farm& requiredFarm() {if(!fields) throw std::logic_error("cGame Farm has not been constructed");return *fields;}
    void construct();void destroy();
    ~World() {destroy();}
    GameMenu& hud() override {return *menu;}
    Farm& farm() override {return requiredFarm();}
    Manufacture& wareDetails() override {return *factories;}
    Market& manufacture() override {return *trade;}
    StateView& farmScene() override {return requiredFarm();}
    StateView& tradeScene() override {return *trade;}
    Manufacture& manufactureScene() override {return *factories;}
    View& sceneOverlay() override {return *buyBuilding;}
    View& selectionOverlay() override {return *buySeeds;}
    View& warehouseTip() override {return *stockTip;}
    bool warehouseTipOpen() const override {return stockTip->opened;}
    void showWarehouseTip() override {stockTip->open();}void hideWarehouseTip() override {stockTip->armHide();}void resetWarehouseTip() override {stockTip->close();}
    bool inputBlocked() const override {return fields->seedMoving || fields->sprinkleMoving;}
    int money() override {return profile.ints["Money"];}int money() const override {return profile.ints["Money"];}
    void setMoney(int value) override {game.setMoney(value);}
    int warehouseValue() const override {return signedBits(Warehouse::instance()->investment());}int wareValue() const override {return signedBits(water->investment());}
    int characterValue() const override {return signedBits(mow->investment());}int farmValue() const override {return signedBits(dig->investment());}
    int wareLevel() const override {return Warehouse::instance()->level();}
    unsigned warehouseFree() override {return Warehouse::instance()->freeSpace();}unsigned warehouseFree() const override {return Warehouse::instance()->freeSpace();}
    unsigned warehouseCapacity(int level) const override {return Warehouse::instance()->levels.at(unsigned(level)).capacity;}
    unsigned manufactureProductCount(unsigned index) override {return trade->goods(index);}bool manufactureProductAvailable(unsigned index) override {return trade->available(index);}
    bool mainCharacterPresent() const override {return actor!=nullptr;}
    bool toolRunning(unsigned slot) const override {const unsigned kinds[]={2,6,1,0,4};return actor->sounds().channel(kinds[slot])!=nullptr;}
    void resumeTool(unsigned kind) override {actor->sounds().play(kind);}
    void stopTool(unsigned kind) override {switch(kind) {case 2:actor->sounds().stopWatering();break;case 6:actor->sounds().stop(6);break;case 1:actor->sounds().stopTractor();break;case 0:actor->sounds().stopShovel();break;case 4:actor->sounds().stopScythe();break;default:throw std::logic_error("Invalid HUD sound kind");}}
    GameplayEvent& savedContract(int index) override {return *gameplay->definitions().events().at(unsigned(index));}
    void stepGameplay() override {gameplay->step();}void completeTutorial(unsigned index) override {tips->complete(index);}void openMessages() override {journal->open();}
    WaterSource& well() override {return *water;}Warehouse& warehouse() override {return *Warehouse::instance();}
    std::vector<Control*> farmPlots() override {std::vector<Control*> result;for(auto* area:fields->graph.areas) if(area->areaType==1 && static_cast<FarmField*>(area)->active) result.push_back(area);return result;}
    std::vector<Control*> farmPlants() override {const auto seeds=fields->seedPackages();return {seeds.begin(),seeds.end()};}
    std::vector<Control*> wareItems() override {const auto buildings=factories->buildings();return {buildings.begin(),buildings.end()};}
    double plotWater(Control& plot) override {return dynamic_cast<FarmField&>(plot).water();}
    float plotWaterLimit(Control& plot) const override {return dynamic_cast<FarmField&>(plot).waterCapacity;}
    int plotKind(Control& plot) override {return signedBits(dynamic_cast<FarmField&>(plot).actionState());}
    int plantState(Control& plant) override {return signedBits(dynamic_cast<SeedPackage&>(plant).status());}
    int wareItemUpgrade(Control& item) override {return dynamic_cast<ProductionBuilding&>(item).level();}
    unsigned wareItemFlags(Control& item) const override {return dynamic_cast<ProductionBuilding&>(item).flags;}
    int anchorState(Control& anchor) const override {return signedBits(dynamic_cast<ContractLetterSlot&>(anchor).letterState);}
    unsigned parentProductCount(Control& anchor) override {return dynamic_cast<ProductionBuilding&>(*anchor.parent()).productionCount();}
    int wellUpgradeLevel() override {return water->level();}int mowerUpgradeLevel() override {return mow->level();}
    double wellCapacity() override {return water->capacity();}
    int characterHeldType() override {return actor->heldType();}float characterHeldWater() override {return actor->heldWater();}
    int characterActivity() override {return signedBits(actor->visual().activity);}bool characterToolFlag() override {return actor->productCapacity();}
    unsigned warehouseProductCount(unsigned index) override {return Warehouse::instance()->goods(index);}
    bool seedMoveActive() const override {return fields->seedMoving;}bool wareSelectionPresent() const override {return factories->movingBuilding()!=nullptr;}
    bool seedSelectionMoving() const override {return buySeeds->movement!=0;}bool buildingSelectionMoving() const override {return buyBuilding->movement!=0;}
    View& contractConfirm() override {return *confirm;}View& contractAim() override {return *aim;}View& contractDialog764() override {return *journal;}View& contractProvide() override {return *provide;}
    TutorialManager& tutorial() override {return *tips;}
    void registerArea(ActionArea& area) override {requiredFarm().registerArea(area);}void queueAreaAction(ActionArea& area,const Point2& point) override {requiredFarm().queueAreaAction(area,point);}
    int warehouseLevel() override {return fields->warehouseLevel();}int digBuildingLevel() override {return fields->digBuildingLevel();}int mowBuildingLevel() override {return fields->mowBuildingLevel();}int waterSourceLevel() override {return fields->waterSourceLevel();}
    bool cursorOverAreaType(unsigned kind,const ActionArea* area) override {return fields->cursorOverAreaType(kind,area);}
    void refreshMarketStocks() override {trade->refresh();}void refreshWarehouseTip() override {stockTip->refresh();}
    void productExpense(unsigned index) override {effects.productExpense(index);}void recordSale(unsigned amount,float time) override {sales.record(amount,time);}
    BuyBuilding& purchasePanel() override {return *buyBuilding;}
    unsigned warehouseGoods(unsigned index) override {
        // 00467fa0 reads the profile directly. Manufacture restores its houses
        // before the Warehouse singleton is constructed (00413210).
        return index<19 ? profile.uints["WGoodCount"+std::to_string(index)]:0;
    }
    void giveWarehouse(unsigned kind,unsigned count,bool refresh,bool preview) override {Warehouse::instance()->give(kind,count,refresh,preview);}
    void takeWarehouse(unsigned kind,unsigned count,bool refresh,bool preview) override {Warehouse::instance()->take(kind,count,refresh,preview);}
    void chooseSeed(unsigned kind) override {fields->createSeed(kind);}
    bool tutorialActive() const override {return tips->active();}bool tutorialStepActive(unsigned index) const override {return tips->activeStep(int(index));}
    bool tutorialCanShow(unsigned index) const override {return tips->canShow(index);}void showTutorial(unsigned index,View& anchor) override {tips->showAction(index,dynamic_cast<Control*>(&anchor));}
    bool worldTransitioning() const override {return fields->movement || factories->movement || trade->movement;}
    float elapsed() const override {return GameTimer::instance()->elapsed();}
    void changeStock(unsigned kind,unsigned count,bool refresh,bool preview) override {Warehouse::instance()->take(kind,count,refresh,preview);}
    void removeStatus(ContractStatus& status) override {menu->removeContract(status);}void notify(const WString& text,bool read) override {menu->notify(text,read);}
    void openReadyDialog(ContractStatus& status) override {confirm->start(status);}void openRequirementsDialog(ContractStatus& status) override {aim->start(status);}void openResultDialog(ContractStatus& status) override {fail->start(status);}
    void playUpgradeSound() override {upgrades.playSound();}
    bool contractAvailable() const override {return fields->hasFreeLetter();}
    void execute(GameplayEvent&) override;
    Upgrade* selectedUpgrade(unsigned index) {switch(index) {case 11:return mow;case 12:return store;case 13:return water;case 14:return dig;default:return factories->focusedBuilding();}}
};
void Game::World::construct() {
    gameplay=std::make_unique<Gameplay>(c.resources,profile,c.random,*this);
    tips=std::make_unique<TutorialManager>(tutorialContext);
    game.loadXML(*c.resources.loadSettingsXML(String("game.xml")));
    stockTip=new WarehouseTip(&game,warehouseTipContext);
    if(!c.pauseContext) throw std::logic_error("cGame requires an engine-owned pause context");
    auto& pause=c.pauseContext(game);
    hudContext=std::make_unique<GameMenuContext>(GameMenuContext{c.resources,c.settings,c.strings,c.input,c.interaction,profile,configuration,c.sound,*this,pause,*this,c.views,c.controls});
    menu=new GameMenu(&game,*hudContext);
    farmContext=std::make_unique<FarmContext>(FarmContext{c.resources,c.settings,c.strings,c.input,c.interaction,profile,configuration,c.random,c.sound,*tips,*this,c.views,c.controls});
    fields=new Farm(menu,*farmContext);
    marketContext=std::make_unique<MarketContext>(MarketContext{c.resources,c.settings,c.strings,c.input,c.interaction,profile,configuration,*gameplay,c.random,c.sound,*tips,*this,c.views,c.controls});
    trade=new Market(menu,*marketContext);factories=new Manufacture(menu,manufactureContext);
    fieldTip=std::make_unique<FieldTip>(nullptr,fieldTipContext);
    store=new Warehouse(&fields->movingPicture(),warehouseContext);
    digContext=std::make_unique<DigBuildingContext>(DigBuildingContext{areaContext,upgrades,configuration,*fields});
    waterContext=std::make_unique<WaterSourceContext>(WaterSourceContext{areaContext,upgrades,configuration,*fields});
    mowContext=std::make_unique<MowBuildingContext>(MowBuildingContext{areaContext,upgrades,configuration,profile,*fields});
    dig=new DigBuilding(&fields->movingPicture(),*digContext);water=new WaterSource(&fields->movingPicture(),*waterContext);mow=new MowBuilding(&fields->movingPicture(),*mowContext);
    menu->refreshWareAvailability();menu->refreshMarketAvailability();
    persistence=std::make_unique<GamePersistence>(c.files,c.profile,*fields,*menu,*gameplay,sales);persistence->restore();
    personContext=std::make_unique<PersonContext>(PersonContext{c.resources,c.input,c.interaction,configuration,profile,c.sound,*fields,*fields,c.views,c.controls});
    actor=new Person(&fields->movingPicture(),*personContext);
    new ElasticAnimation(&fields->movingPicture(),c.resources,c.sound,c.views,c.controls);
    buyBuildingContext=std::make_unique<BuyBuildingContext>(BuyBuildingContext{c.resources,c.settings,c.strings,c.input,c.interaction,profile,configuration,c.sound,*factories,c.views,c.controls});
    buyBuilding=new BuyBuilding(&game,*buyBuildingContext);buySeeds=new BuySeeds(&game,buySeedsContext);
    journalContext=std::make_unique<JournalContext>(JournalContext{c.resources,c.settings,c.strings,c.input,c.interaction,c.views,c.controls});
    journal=std::make_unique<Journal>(nullptr,*journalContext);
    dialogContext=std::make_unique<ContractDialogContext>(ContractDialogContext{c.resources,c.settings,c.strings,c.input,c.interaction,configuration,*menu,c.views,c.controls});
    provide=std::make_unique<ContractProvide>(nullptr,*dialogContext,*fields);
    aim=std::make_unique<ContractAim>(nullptr,*dialogContext);confirm=std::make_unique<ContractPassConfirm>(nullptr,*dialogContext);fail=std::make_unique<ContractFail>(nullptr,*dialogContext);
    help=std::make_unique<HelpView>(nullptr,helpContext);awards=std::make_unique<GameAwardManager>(awardContext);
    menu->initializeScene();fields->seedMoving=false;fields->selected=nullptr;fields->sprinkleMoving=false;complete=true;
}
void Game::World::destroy() {
    configuration.releaseUpgradeSound();
    awards.reset();tips.reset();help.reset();provide.reset();aim.reset();confirm.reset();fail.reset();journal.reset();fieldTip.reset();
    if(complete) {persistence->save();complete=false;}
    persistence.reset();gameplay.reset();upgrades.clearSound();
    // Contexts and all virtual world bases must remain alive during UI teardown.
    while(!game.children().empty()) delete *game.children().begin();
}
void Game::World::execute(GameplayEvent& event) {
    const auto note=[&] {if(!event.note.empty()) menu->notify(event.note,false);};
    switch(event.type) {
    case GameplayEventType::DepotUpgrade: {note();const auto& value=std::get<DepotUpgradeEvent>(event.data);profile.uints["UpAvail"+std::to_string(value.depotType)]=value.upgrade;
        if(visible(*trade)) trade->refresh();if(auto* upgrade=selectedUpgrade(value.depotType)) {upgrade->setLevel(upgrade->level(),false);if(value.depotType<11 || value.depotType>14) {auto* house=dynamic_cast<ProductionBuilding*>(upgrade);factories->focus(nullptr,true);factories->focus(house,true);}}
        menu->refreshWareAvailability();menu->refreshMarketAvailability();break;}
    case GameplayEventType::SeedAvailable: {note();const auto& value=std::get<SeedAvailableEvent>(event.data);profile.uints["SeedAvail"+std::to_string(value.seedType)]=1;if(visible(*trade)) trade->refresh();fields->refreshSeedButton();if(visible(*stockTip)) stockTip->refresh();break;}
    case GameplayEventType::Money: {const auto& value=std::get<RandomValueEvent>(event.data);if(!(originalMultiply(c.random.next(),3.0518509447574615e-05f)<=value.randomFactor)) break;const int before=money(),after=signedBits(unsigned(before)+unsigned(value.value));if(after>=0) {note();setMoney(after);}break;}
    case GameplayEventType::DestroyField: {note();const auto& value=std::get<RandomValueEvent>(event.data);fields->destroyFields(value.value,value.randomFactor);break;}
    case GameplayEventType::DestroyDepot:break; // original vtable005f98f4 is genuine RET004bd820
    case GameplayEventType::Contract: {const auto& value=std::get<ContractEvent>(event.data);if(originalMultiply(c.random.next(),3.0518509447574615e-05f)<=value.randomFactor && fields->hasFreeLetter()) fields->addLetter(event);break;}
    case GameplayEventType::Popularity: {note();const auto& value=std::get<PopularityEvent>(event.data);for(const auto& item:value.values) {profile.ints["MEffPrice"+std::to_string(item.productType)]=item.value;profile.floats["MSellTime"+std::to_string(item.productType)]=-1;}if(!value.values.empty()) trade->refresh();break;}
    case GameplayEventType::FieldUpgrade:note();fields->setAvailableUpgrade(std::get<FieldUpgradeEvent>(event.data).upgradeIndex);break;
    case GameplayEventType::Trigger: {const auto& value=std::get<TriggerEvent>(event.data);profile.ints["mTrigger"]=value.money;profile.ints["wlTrigger"]=value.wareLevel;break;}
    case GameplayEventType::End:profile.booleans["MainGame"]=false;break;
    }
}
Game::Game(GameContext& context):View(nullptr,context.views),GameLifetime(this),world_(std::make_unique<World>(*this,context)) {world_->construct();}
Game::~Game() {world_.reset();currentGame=nullptr;}
void Game::loadXML(const XmlNode& node) {View::loadXML(node);world_->configuration.loadXML(node);}
int Game::money() const {return world_->money();}
void Game::setMoney(int value) {
    if(value<0) value=0;auto& w=*world_;const int before=w.profile.ints["Money"];w.profile.ints["Money"]=value;
    refreshUpgrades();w.menu->refreshMoney();
    Upgrade* tools[]={w.dig,w.mow,w.store,w.water};for(auto* tool:tools) tool->setLevel(tool->level(),false);
    w.fields->refreshUpgradeButton();if(value<before) w.effects.moneyExpense(unsigned(before)-unsigned(value));else if(before<value) w.effects.moneyIncome(unsigned(value)-unsigned(before));
}
void Game::refreshUpgrades() {
    auto& w=*world_;
    for(unsigned i=0;i<15;++i) {
        if(i<11) {
            // Native0040bec6 passes the unadjusted focused Picture pointer.
            if(auto* house=w.factories->focusedBuilding()) {
                refreshViewUpgradeAlias(w.profile,*house);
                if(!w.tips->activeStep(34)) {w.factories->focus(nullptr,true);w.factories->focus(house,false);}
            }
        } else if(auto* upgrade=w.selectedUpgrade(i)) upgrade->setLevel(upgrade->level(),false);
    }
}
void Game::cheatMoney() {setMoney(signedBits(unsigned(money())+1000U));}
void Game::cheatUpgrades() {
    auto& w=*world_;for(unsigned i=0;i<15;++i) w.profile.uints["UpAvail"+std::to_string(i)]=i==13 ? 2U:3U;
    refreshUpgrades();w.menu->refreshWareAvailability();
}
void Game::cheatSeeds() {for(unsigned i=0;i<8;++i) world_->profile.uints["SeedAvail"+std::to_string(i)]=1;}
void Game::cheatProducts() {for(unsigned i=0;i<19;++i) world_->store->give(i,2,true,false);world_->menu->refreshMarketAvailability();}
void Game::save() {world_->persistence->save();}
Farm& Game::farm() {return *world_->fields;}Market& Game::market() {return *world_->trade;}Manufacture& Game::manufacture() {return *world_->factories;}GameMenu& Game::hud() {return *world_->menu;}Person& Game::person() {return *world_->actor;}
Gameplay& Game::gameplay() {return *world_->gameplay;}SalesHistory& Game::sales() {return world_->sales;}GameConfiguration& Game::configuration() {return world_->configuration;}TutorialManager& Game::tutorial() {return *world_->tips;}
}
