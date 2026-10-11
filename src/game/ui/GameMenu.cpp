#include "GameMenu.h"
#include "ResourceManager.h"
#include "XmlDocument.h"
#include "ProfileProperties.h"
#include "SoundManager.h"
#include "Sound.h"
#include "OriginalArithmetic.h"
#include "StringConversion.h"
#include <sstream>
#include <stdexcept>
namespace farmar::recovered {
namespace {
GameMenu* currentGameMenu=nullptr;
bool shown(const View& v) {return v.state().shown && v.state().parentShown;}
bool enabled(const View& v) {return v.state().enabled && v.state().parentEnabled;}
bool sameColor(const Color& a,const Color& b) {return a.r==b.r && a.g==b.g && a.b==b.b && a.a==b.a;}
const unsigned toolKinds[]={2,6,1,0,4};
// Original GameMenu's warehouse substrate is a Button with actual special
// hover/enable hooks; the original 636-byte object's primary table is 005fa3f0.
class WarehouseButton final:public Button {
public:
    WarehouseButton(View* parent,GameMenuContext& c):Button(parent,c.resources,c.input,c.interaction,c.views,c.controls),context_(c) {}
    void onEnabled() override { // 004bd8a0
        ButtonPrototype::onEnabled();const auto point=context_.interaction.cursor();
        if(pointInRect(point)) {setState(ButtonState::Hovered);hovered();}
        else {setState(ButtonState::Normal);unhovered();}
    }
    void onDisabled() override {ButtonPrototype::onDisabled();} // 004bd830
    void hovered() override { // 004bd8f0
        ButtonPrototype::hovered();GameMenu::instance()->warehouseLighter().show();
        if(!context_.world.warehouseTipOpen()) context_.world.showWarehouseTip();
    }
    void unhovered() override { // 004bd920
        ButtonPrototype::unhovered();GameMenu::instance()->warehouseLighter().hide();
        if(context_.world.warehouseTipOpen()) context_.world.hideWarehouseTip();
    }
private:GameMenuContext& context_;
};
}
GameMenuLifetime::GameMenuLifetime(GameMenu* value) {if(currentGameMenu) throw "Object already exist.";currentGameMenu=value;}
GameMenuLifetime::~GameMenuLifetime() {currentGameMenu=nullptr;}
GameMenu* GameMenu::instance() noexcept {return currentGameMenu;}
GameMenu::GameMenu(View* parent,GameMenuContext& c)
    :GameMenuLifetime(this),MenuView(parent,c.resources,c.settings,c.strings,c.views,c.controls),context_(c) {
    const auto tutorial=[&c] {return c.contracts.tutorialActive();};
    main_=new ExtendedTextAnimationButton(this,c.resources,c.settings,c.strings,c.input,c.interaction,tutorial,c.views,c.controls);
    market_=new ExtendedTextAnimationButton(this,c.resources,c.settings,c.strings,c.input,c.interaction,tutorial,c.views,c.controls);
    farm_=new ExtendedTextAnimationButton(this,c.resources,c.settings,c.strings,c.input,c.interaction,tutorial,c.views,c.controls);
    ware_=new ExtendedTextAnimationButton(this,c.resources,c.settings,c.strings,c.input,c.interaction,tutorial,c.views,c.controls);
    warehouse_=new WarehouseButton(this,c);
    progress_=new ProgressBar(this,c.resources,c.views,c.controls);
    lighter_=new Picture(this,c.resources,c.views,c.controls);
    money_=new TextLine(this,c.resources,c.settings,c.strings,c.views,c.controls);
    journal_=new ExtendedAnimationButton(this,c.resources,c.settings,c.strings,c.input,c.interaction,tutorial,c.views,c.controls);
    for(unsigned n=0;n<3;++n) status_[n]=new ContractStatus(this,n,c.resources,c.settings,c.strings,c.input,c.interaction,c.profile,c.configuration,c.sound,c.contracts,c.views,c.controls);
    {const auto xml=c.resources.loadSettingsXML(String("UI/gameUI/GameMenu.xml"));loadXML(*xml);}
    main_->clickEvent=&mainClick;market_->clickEvent=&marketClick;farm_->clickEvent=&farmClick;ware_->clickEvent=&wareClick;journal_->clickEvent=&journalClick;
    for(unsigned n=0;n<3;++n) {const auto saved=c.profile.ints["task"+std::to_string(n)];if(saved!=-1) {status_[n]->assign(c.world.savedContract(saved),true);++contractCount;}}
    pause_=new GamePauseMenu(nullptr,c.pause);
}
GameMenu::~GameMenu() {
    if(changeSound_) changeSound_->releaseReference();if(messageSound_) messageSound_->releaseReference();delete pause_;
    // Native singleton is cleared before MenuView destroys its owned children.
    currentGameMenu=nullptr;
}
void GameMenu::loadXML(const XmlNode& node) {
    MenuView::loadXML(node);normalColor=journal_->geometry().color;
    for(const auto& pair:std::initializer_list<std::pair<const char*,View*>>{{"MainMenuButton",main_},{"MarketButton",market_},{"FarmButton",farm_},{"ProducingButton",ware_},{"MoneyText",money_},{"JournalButton",journal_}}) {
        XmlNode* section=nullptr;if(node.section(String(pair.first),section)) pair.second->loadXML(*section);
    }
    const auto color=node.attribute(WString(L"JournalBlinkColor"));if(!color.empty()) {unsigned value=0;std::wistringstream stream(color.c_str());stream>>std::hex>>value;blinkColor=Color::fromARGB(value);}
    const auto period=node.attribute(WString(L"JournalBlinkPeriod"));if(!period.empty()) {std::wistringstream stream(period.c_str());stream>>blinkPeriod;}
    for(const auto& pair:std::initializer_list<std::pair<const char*,View*>>{{"WareLoadingSubstrain",warehouse_},{"WareLoading",progress_},{"WareLoadingLighter",lighter_}}) {
        XmlNode* section=nullptr;if(node.section(String(pair.first),section)) pair.second->loadXML(*section);
    }
    // XML uses unsigned formatter 0041db60; runtime refresh uses signed 004bda10.
    money_->setString(WString(std::to_wstring(static_cast<unsigned>(context_.world.money()))));XmlNode* places=nullptr;
    if(node.section(String("ContractStatusPlaces"),places)) for(unsigned n=0;n<std::min<std::size_t>(3,places->children().size());++n) {
        XmlNode* place=nullptr;if(!places->section(n,place)) continue;Point2 offset{};const auto value=place->attribute(WString(L"Place"));std::wistringstream stream(value.c_str());stream>>offset[0]>>offset[1];
        auto& substrate=status_[n]->substrate();substrate.setPosition({float(originalAdd(substrate.geometry().x,offset[0])),float(originalAdd(substrate.geometry().y,offset[1]))});
    }
    // Original reload overwrites these references without releasing the old ones.
    const auto change=toNarrow(node.attribute(WString(L"ChangeSound")));if(!change.empty()) changeSound_=context_.resources.getSound(change);
    const auto message=toNarrow(node.attribute(WString(L"NewMessageSound")));if(!message.empty()) messageSound_=context_.resources.getSound(message);
}
void GameMenu::refreshMoney() {money_->setString(WString(std::to_wstring(context_.world.money())));}
void GameMenu::refreshWarehouse() {
    auto& w=context_.world;const auto level=w.wareLevel();const auto capacity=w.warehouseCapacity(level),free=w.warehouseFree();
    progress_->current=float(originalDivide(originalMultiply(originalUnsigned(capacity-free),100),originalUnsigned(capacity)));progress_->updateLid();
}
void GameMenu::stockChanged(bool preview) {refreshMarketAvailability();for(auto* status:status_) if(status->active) status->stockChanged(preview);}
void GameMenu::suspendButtons() {
    resumeSceneButtons={enabled(*market_),enabled(*farm_),enabled(*ware_)};
    for(auto* button:std::array<View*,5>{main_,market_,farm_,ware_,journal_}) button->disable();
}
void GameMenu::resumeButtons() {
    main_->enable();journal_->enable();const std::array<View*,3> buttons{market_,farm_,ware_};for(unsigned n=0;n<3;++n) if(resumeSceneButtons[n]) buttons[n]->enable();
}
void GameMenu::initializeScene() {context_.world.farm().show();farm_->disable();refreshWareAvailability();refreshMarketAvailability();}
bool GameMenu::transitioning() const {auto& w=context_.world;return w.farm().movement || w.wareDetails().movement || w.manufacture().movement;}
void GameMenu::onAbsoluteEnabled() {
    View::onAbsoluteEnabled();if(context_.world.mainCharacterPresent()) for(unsigned n=0;n<5;++n) if(resumeTools[n]) context_.world.resumeTool(toolKinds[n]);
    context_.input.bind(1,&escapeInput,this);
}
void GameMenu::onAbsoluteDisabled() {
    View::onAbsoluteDisabled();if(context_.world.mainCharacterPresent()) {
        for(unsigned n=0;n<5;++n) resumeTools[n]=context_.world.toolRunning(n);
        for(unsigned n=0;n<5;++n) context_.world.stopTool(toolKinds[n]);
    } else resumeTools.fill(false);
    context_.input.unbind(1,&escapeInput,this);
}
void GameMenu::activateContract(GameplayEvent& event) {
    if(contractCount==3) return;
    for(auto* status:status_) if(!status->active) {status->assign(event,false);++contractCount;return;}
    // Original uses an unspecified pointer in inconsistent count/active state.
    throw std::logic_error("Original GameMenu contract count is inconsistent");
}
void GameMenu::removeContract(ContractStatus& value) {for(auto* status:status_) if(status==&value) {--contractCount;return;}}
void GameMenu::assignMessages(const std::vector<GameMessage>& values) {
    messages_=values;hasUnread=false;for(const auto& item:messages_) hasUnread=hasUnread || !item.read;
    if(!messages_.empty()) journal_->show();if(hasUnread) blinkRemaining=blinkPeriod;
}
void GameMenu::notify(const WString& text,bool read) {
    if(!read && messageSound_) context_.sound.clickSound(messageSound_);
    messages_.push_back({text,read});if(!read && !hasUnread) {hasUnread=true;blinkRemaining=blinkPeriod;}
    if(!shown(*journal_)) journal_->show();
}
bool GameMenu::upgradesAvailable() {for(unsigned n=0;n<11;++n) if(context_.profile.uints["UpAvail"+std::to_string(n)]!=0xffffffffU) return true;return false;}
void GameMenu::refreshWareAvailability() {
    auto& detail=context_.world.wareDetails();if(!enabled(*ware_) && (!shown(detail) || detail.movement)) if(upgradesAvailable()) ware_->enable();
}
void GameMenu::refreshMarketAvailability() {
    if(enabled(*market_)) return;auto& w=context_.world;if(shown(w.manufacture()) && !w.manufacture().movement) return;
    if(w.manufactureProductCount(0)+context_.profile.uints["WGoodCount0"]!=0) {market_->enable();return;}
    for(unsigned n=1;n<19;++n) if(w.manufactureProductAvailable(n)) {market_->enable();return;}
}
void GameMenu::finishSwitch() {
    if(changeSound_) context_.sound.clickSound(changeSound_);
    context_.world.sceneOverlay().hide();context_.world.selectionOverlay().hide();context_.world.resetWarehouseTip();context_.world.warehouseTip().hide();
}
void GameMenu::switchFarm() {
    auto& w=context_.world;auto& detail=w.wareDetails();auto& manufacture=w.manufacture();auto& outgoing=shown(detail) ? detail:manufacture;auto& unused=shown(detail) ? manufacture:detail;
    if(shown(detail)) {outgoing.move(1,{-1,0},{799,0});w.farm().move(2,{-801,0},{-1,0});}
    else {outgoing.move(1,{-1,0},{-801,0});w.farm().move(2,{799,0},{-1,0});}
    unused.move(0,{-1,0},{-1,0});unused.hide();market_->enable();farm_->disable();refreshWareAvailability();finishSwitch();
}
void GameMenu::switchMarket() {
    auto& w=context_.world;auto& farm=w.farm();auto& detail=w.wareDetails();auto& outgoing=shown(farm) ? farm:detail;auto& unused=shown(farm) ? detail:farm;
    outgoing.move(1,{-1,0},{799,0});w.manufacture().move(2,{-801,0},{-1,0});unused.move(0,{-1,0},{-1,0});unused.hide();
    market_->disable();farm_->enable();if(upgradesAvailable()) ware_->enable();else ware_->disable();finishSwitch();
}
void GameMenu::switchWare() {
    auto& w=context_.world;auto& farm=w.farm();auto& manufacture=w.manufacture();auto& outgoing=shown(farm) ? farm:manufacture;auto& unused=shown(farm) ? manufacture:farm;
    outgoing.move(1,{-1,0},{-801,0});w.wareDetails().move(2,{799,0},{-1,0});unused.move(0,{-1,0},{-1,0});unused.hide();
    market_->enable();farm_->enable();ware_->disable();finishSwitch();
}
void GameMenu::requestPause() {
    if(context_.world.inputBlocked()) return;context_.pause.game.disable();context_.pause.game.absoluteDisable();pause_->showTransparently();pause_->enable();
}
void GameMenu::escape() {if(!transitioning()) requestPause();}
void GameMenu::showMessages() {
    if(context_.contracts.tutorialActive() && !context_.contracts.tutorialStepActive(0x27)) return;
    context_.world.completeTutorial(0x27);context_.world.openMessages();
}
void GameMenu::absoluteStep() {
    if(!uiElapsedTime()) throw std::logic_error("UI timer is not bound");const float elapsed=uiElapsedTime()();auto& p=context_.profile;auto& w=context_.world;
    p.floats["RunTime"]=float(originalAdd(p.floats["RunTime"],elapsed));
    if(!transitioning() && !gameplayPaused) {
        bool blocked=false;const auto trigger=p.ints["mTrigger"];
        if(trigger) {const auto money=w.money(),warehouse=w.warehouseValue(),ware=w.wareValue(),character=w.characterValue(),farm=w.farmValue();
            const auto sum=std::int32_t(unsigned(money)+unsigned(warehouse)+unsigned(ware)+unsigned(character)+unsigned(farm));
            if(trigger<=sum) p.ints["mTrigger"]=0;else blocked=true;
        }
        if(!blocked) {const auto level=p.ints["wlTrigger"];if(level!=-1) {if(w.wareLevel()<level) blocked=true;else p.ints["wlTrigger"]=-1;}}
        const float gameTime=p.floats["GameTime"];if(!blocked) {p.floats["GameTime"]=float(originalAdd(gameTime,elapsed));w.stepGameplay();}
    }
    if(hasUnread) {
        if(journal_->buttonState()==ButtonState::Hovered) {journal_->setColor(normalColor);blinkRemaining=blinkPeriod;}
        else {const auto left=originalSubtract(blinkRemaining,elapsed);blinkRemaining=float(left);if(left<=0) {blinkRemaining=blinkPeriod;journal_->setColor(sameColor(journal_->geometry().color,blinkColor) ? normalColor:blinkColor);}}
    }
    if(!w.warehouseFree() && warehouse_->buttonState()!=ButtonState::Hovered && warehouse_->buttonState()!=ButtonState::Pressed) {
        const auto left=originalSubtract(lighterRemaining,elapsed);lighterRemaining=float(left);if(left<=0) {lighterRemaining=0.5f;if(shown(*lighter_)) lighter_->hide();else lighter_->show();}
    }
    const auto visible=[](StateView& view) {return !view.movement && shown(view);};auto& tutorial=context_.contracts;
    if(visible(w.farm()) && enabled(*market_) && tutorial.tutorialCanShow(6) && !tutorial.tutorialCanShow(5)) tutorial.showTutorial(6,*market_);
    if(visible(w.manufacture()) && enabled(*farm_) && tutorial.tutorialCanShow(7) && !tutorial.tutorialCanShow(0x2d)) tutorial.showTutorial(7,*farm_);
    if(tutorial.tutorialCanShow(0x1e) && (visible(w.farm()) || visible(w.manufacture())) && enabled(*ware_)) tutorial.showTutorial(0x1e,*ware_);
    if(tutorial.tutorialCanShow(0x27) && (visible(w.farm()) || visible(w.manufacture()) || visible(w.wareDetails())) && hasUnread && messages_.size()>2) tutorial.showTutorial(0x27,*journal_);
}
void GameMenu::mainClick(ButtonPrototype*) {instance()->requestPause();}
void GameMenu::marketClick(ButtonPrototype*) {auto& m=*instance();auto& t=m.context_.contracts;if(m.context_.world.inputBlocked()) return;if(t.tutorialActive() && !t.tutorialStepActive(6) && !t.tutorialStepActive(0x29) && !t.tutorialStepActive(0x2c)) return;m.switchMarket();}
void GameMenu::farmClick(ButtonPrototype*) {auto& m=*instance();auto& t=m.context_.contracts;if(t.tutorialActive() && !t.tutorialStepActive(7)) return;m.switchFarm();}
void GameMenu::wareClick(ButtonPrototype*) {auto& m=*instance();auto& t=m.context_.contracts;if(m.context_.world.inputBlocked()) return;if(t.tutorialActive() && !t.tutorialStepActive(0x1e) && !t.tutorialStepActive(0x29) && !t.tutorialStepActive(0x2c)) return;m.switchWare();}
void GameMenu::journalClick(ButtonPrototype*) {instance()->showMessages();}
void GameMenu::escapeInput(void* value) {static_cast<GameMenu*>(value)->escape();}
}
