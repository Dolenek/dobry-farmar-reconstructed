#include "Manufacture.h"
#include "GameMenu.h"
#include "TutorialManager.h"
#include "SoundManager.h"
#include "Sound.h"
#include "ResourceManager.h"
#include "XmlDocument.h"
#include "StringConversion.h"
#include <sstream>
#include <cstring>
#include <string>
namespace farmar::recovered {
namespace {
Manufacture* manufactureSingleton=nullptr;
ProductionBuilding* movingProductionBuilding=nullptr;
bool enabled(const View& view) {return view.state().enabled && view.state().parentEnabled;}
bool rectangleHit(const Control& control,const Point2& point) {
    // 00417910 spills left/top, retains the right sum, spills bottom.
    const auto& g=control.geometry();const float left=float(originalSubtract(g.x,originalMultiply(g.width,0.5))),top=float(originalSubtract(g.y,originalMultiply(g.height,0.5)));
    const auto right=originalAdd(left,g.width);const float bottom=float(originalAdd(top,g.height));
    return left<=point[0] && point[0]<=right && top<=point[1] && point[1]<=bottom;
}
WString buildingName(unsigned type) {
    // Literal UTF-16 table initialized by 00496520, before localization.
    static const wchar_t* names[]={L"\u041e\u0432\u0447\u0430\u0440\u043d\u044f",L"\u041a\u0443\u0440\u044f\u0442\u043d\u0438\u043a",L"\u041a\u043e\u0440\u043e\u0432\u043d\u0438\u043a",L"\u0421\u0432\u0438\u043d\u0430\u0440\u043d\u0438\u043a",L"\u0421\u044b\u0440\u043e\u0432\u0430\u0440\u043d\u044f",L"\u041c\u0430\u0441\u043b\u043e\u0431\u043e\u0439\u043a\u0430",L"\u041f\u0440\u044f\u0434\u0438\u043b\u044c\u043d\u044f",L"\u0418\u043d\u043a\u0443\u0431\u0430\u0442\u043e\u0440",L"\u0422\u043a\u0430\u043b\u044c\u043d\u044f",L"\u041a\u043e\u043d\u0434\u0438\u0442\u0435\u0440\u0441\u043a\u0430\u044f",L"\u041c\u0435\u043b\u044c\u043d\u0438\u0446\u0430"};
    if(type>=11) throw "unknown building";return WString(names[type]);
}
void child(View& view,const XmlNode& node,const char* name) {XmlNode* definition=nullptr;if(node.section(String(name),definition)) view.loadXML(*definition);}
void rectangle(const XmlNode& node,const wchar_t* name,ProductionRectangle& output) {
    const auto text=node.attribute(WString(name));if(!text.empty()) {std::wistringstream stream(text.c_str());for(unsigned i=0;i<4;++i) {if(i) stream.ignore(1);stream>>output[i];}}
}
class DemolitionEffect final:public Picture {
public:
    DemolitionEffect(View* parent,const Point2& position,ManufactureContext& c):Picture(parent,c.resources,c.views,c.controls) {
        loadXML(*c.resources.loadSettingsXML(String("manufacture/depotExplos.xml")));setPosition(position); // 00493310 / 00492f50 -> Picture XML
    }
    void onAnimationEnded() override {registry_.requestDiscard(this,String(".\\Game\\Manufacture\\DepotExplos.cpp48"));} // 00493820
};
}
Manufacture* Manufacture::instance() noexcept {return manufactureSingleton;}
Manufacture::Manufacture(View* parent,ManufactureContext& c):StateView(parent,c.resources,c.settings,c.strings,c.views,c.controls),context_(c),
    buildingContext_{c.resources,c.settings,c.strings,c.input,c.interaction,c.profile,c.configuration,c.sound,c.upgrades,*this,c.views,c.controls} {
    if(manufactureSingleton) throw "Object already exist.";manufactureSingleton=this;
    // Native +60 is StateView's fixed origin; houses use moving Picture +38.
    currentText_=new TextLine(&origin(),c.resources,c.settings,c.strings,c.views,c.controls);
    currentPicture_=new Picture(&origin(),c.resources,c.views,c.controls);
    build_=new ExtendedTextAnimationButton(&origin(),c.resources,c.settings,c.strings,c.input,c.interaction,[this]{return tutorial().active();},c.views,c.controls);
    destroy_=new ExtendedAnimationButton(&origin(),c.resources,c.settings,c.strings,c.input,c.interaction,[this]{return tutorial().active();},c.views,c.controls);
    upgrade_=new ExtendedAnimationButton(&origin(),c.resources,c.settings,c.strings,c.input,c.interaction,[this]{return tutorial().active();},c.views,c.controls);
    price_=new TextMoney(&origin(),c.resources,c.settings,c.strings,c.views,c.controls);
    loadXML(*c.resources.loadSettingsXML(String("UI/gameUI/Manufacture.xml")));
    build_->clickEvent=&buildClick;destroy_->clickEvent=&destroyClick;upgrade_->clickEvent=&upgradeClick;
    for(unsigned slot=0;slot<20;++slot) if(c.profile.ints["Upgrade"+std::to_string(slot)]!=-1) create(c.profile.uints["ProdType"+std::to_string(slot)],slot);
    build_->show();upgrade_->hideTips();price_->hide(); // 00499a89 / 00499aa2 target child controls
}
Manufacture::~Manufacture() {
    context_.input.unbind(0x208,&outsidePress,this);context_.input.unbind(0x209,&rightPress,this);if(selectSound_) selectSound_->releaseReference();forbidden.clear();manufactureSingleton=nullptr;
    // Base StateView/View owns both origin controls and moving-picture houses.
    // House teardown retains the global moving-address value, so no virtual
    // world call is made after this scene's service bases are destroyed.
}
void Manufacture::loadXML(const XmlNode& node) {
    StateView::loadXML(node);child(*currentPicture_,node,"CurrentBuildingPic");child(*build_,node,"BuildButton");child(*destroy_,node,"DestroyButton");
    child(*upgrade_,node,"UpgradeButton");child(*price_,node,"UpgradeText");child(*currentText_,node,"CurrentBuildingText");
    const auto sound=toNarrow(node.attribute(WString(L"SelectSound")));if(!sound.empty()) selectSound_=context_.resources.getSound(sound); // native reload retains overwritten reference
    rectangle(node,L"DepotArea",allowed);XmlNode* list=nullptr;
    if(node.section(String("AntiDepotAreaList"),list)) {
        // 00497890 resizes with zero rectangles, then 00497930 appends each
        // parsed rectangle. This is not reserve: a first load has 2*n rows.
        forbidden.resize(list->children().size(),ProductionRectangle{});
        for(unsigned i=0;i<list->children().size();++i) {XmlNode* entry=nullptr;if(list->section(i,entry)) {ProductionRectangle value{};rectangle(*entry,L"Rect",value);forbidden.push_back(value);}}
    }
}
TutorialManager& Manufacture::tutorial() {return context_.world.tutorial();}
GameMenu& Manufacture::hud() {return context_.world.hud();}
ProductionBuilding*& Manufacture::movingBuilding() {return movingProductionBuilding;}
unsigned Manufacture::warehouseGoods(unsigned type) {return context_.world.warehouseGoods(type);}
unsigned Manufacture::warehouseFree() {return context_.world.warehouseFree();}
void Manufacture::giveWarehouse(unsigned type,unsigned amount,bool a,bool b) {context_.world.giveWarehouse(type,amount,a,b);}
void Manufacture::takeWarehouse(unsigned type,unsigned amount,bool a,bool b) {context_.world.takeWarehouse(type,amount,a,b);}
int Manufacture::money() {return context_.world.money();}
void Manufacture::setMoney(int value) {context_.world.setMoney(value);}
bool Manufacture::scenesMoving() {return movement || context_.world.farmScene().movement || context_.world.tradeScene().movement;}
ProductionBuilding* Manufacture::create(unsigned type,int requested) {
    if(count==20) return nullptr;ProductionBuilding* building=nullptr;
    for(unsigned slot=requested==-1 ? 0:static_cast<unsigned>(requested);slot<20;++slot) if(!slots_[slot]) {
        productionKind(type); // original unknown-building exception before allocation
        building=new ProductionBuilding(type,slot,&movingPicture(),buildingContext_);slots_[slot]=building;break;
    }
    ++count; // original increments even if an invalid request found no empty slot
    if(count==20) upgrade_->hideTips();return building; // actual native +98
}
void Manufacture::chooseBuilding(unsigned type,int slot) {create(type,slot);}
std::vector<ProductionBuilding*> Manufacture::buildings() {
    std::vector<ProductionBuilding*> result;for(auto* building:slots_) if(building) result.push_back(building);return result;
}
bool Manufacture::manipulationBlocked() {for(auto* building:slots_) if(building && building->cursorInControls()) return true;return false;}
bool Manufacture::validPlacement(ProductionBuilding& building) {
    if(!productionRectangleContains(allowed,&building)) return false;
    for(const auto& rectangle:forbidden) if(productionRectangleTouches(rectangle,&building)) return false;
    for(auto* other:slots_) if(other && other!=&building && productionMasksOverlap(building,*other,&building.clickMask(),&other->clickMask())) return false;
    return true;
}
void Manufacture::beginManipulation() {
    restoreUpgrade_=enabled(*upgrade_);build_->disable();destroy_->disable();upgrade_->disable();hud().suspendButtons();
}
void Manufacture::endManipulation() {
    build_->enable();if(restoreUpgrade_) upgrade_->enable();else upgrade_->disable();destroy_->enable();hud().resumeButtons();
}
void Manufacture::absoluteStep() {StateView::absoluteStep();}
void Manufacture::onShown() {View::onShown();focus(nullptr,false);}
void Manufacture::refreshHouseControls() {
    for(auto* building:slots_) if(building) building->warehouseQuantity().setString(WString(std::wstring(L"x")+std::to_wstring(warehouseGoods(productionKind(building->buildingType).inputProduct()))));
}
void Manufacture::onEnabled() {
    context_.input.bind(0x208,&outsidePress,this);context_.input.bind(0x209,&rightPress,this);refreshHouseControls();View::onEnabled();
    for(auto* building:slots_) if(building) building->refreshHover();
}
void Manufacture::onDisabled() {context_.input.unbind(0x208,&outsidePress,this);context_.input.unbind(0x209,&rightPress,this);View::onDisabled();}
void Manufacture::remove(ProductionBuilding& building) {
    for(auto& slot:slots_) if(slot==&building) {
        const Point2 position{building.geometry().x,building.geometry().y};focus(nullptr,true);slot=nullptr;
        building.setProductionProgress(0);building.setProductionCount(0);building.setProductionStopped(false);building.setLevel(-1,false);
        const bool wasBuilt=building.built;if(wasBuilt && building.sounds()[4]) context_.sound.clickSound(building.sounds()[4]);
        building.disable();building.hideTips();context_.views.requestDiscard(&building,String(".\\Game\\Manufacture\\Manufacture.cpp437")); // 00498840 uses same 006604e4 map as 0047d0b0
        --count;if(wasBuilt) new DemolitionEffect(this,position,context_);return;
    }
}
void Manufacture::focus(ProductionBuilding* building,bool playSound) {
    if(!building) {currentText_->hide();currentPicture_->hide();destroy_->hideTips();upgrade_->hideTips();price_->hide();}
    else {
        if(focused_!=building && playSound && selectSound_) context_.sound.clickSound(selectSound_);
        destroy_->show();const int current=building->level();if(static_cast<unsigned>(current)>3) throw "bad level";
        currentPicture_->copyAppearance(building->levels[static_cast<unsigned>(current)].appearance);currentPicture_->markTransformDirty();currentPicture_->show();
        currentText_->setString(buildingName(building->buildingType));currentText_->show();
        if(building->canUpgrade(false,true)) {
            const auto cost=context_.configuration.upgradeCost(building->itemType,building->level()+1);price_->setString(WString(std::to_wstring(cost)));price_->show();upgrade_->show();
            if(building->canUpgrade(true,true)) {price_->enable();upgrade_->enable();}else {upgrade_->disable();price_->disable();}
        } else {upgrade_->hideTips();price_->hide();}
    }
    focused_=building;
}
void Manufacture::buildClick(ButtonPrototype*) {
    auto& self=*instance();auto& tips=self.tutorial();if(tips.active() && !tips.activeStep(32)) return;
    self.hud().disable();self.hud().absoluteDisable();self.context_.world.purchasePanel().open(); // 00493c60
}
void Manufacture::destroyClick(ButtonPrototype*) {
    auto& self=*instance();auto& tips=self.tutorial();if(tips.activeStep(38) || tips.activeStep(34) || tips.activeStep(48)) return;
    if(!self.focused_) throw std::logic_error("Original destroy callback requires a focused building");
    self.remove(*self.focused_);const auto count=self.context_.profile.uints["awQDestroy"];self.context_.profile.uints["awQDestroy"]=count+1; // 00499630
}
void Manufacture::upgradeClick(ButtonPrototype*) {
    auto& self=*instance();auto& tips=self.tutorial();if(tips.active() && !tips.activeStep(34)) return;
    auto* building=self.focused_;if(!building) throw std::logic_error("Original upgrade callback requires a focused building");
    const unsigned next=static_cast<unsigned>(building->level()+1);if(next>=4) return;
    unsigned cost=self.context_.configuration.upgradeCost(building->itemType,next);const int currentMoney=self.money();if(tips.activeStep(34)) cost=0;
    const unsigned bits=static_cast<unsigned>(currentMoney)-cost;int remaining;std::memcpy(&remaining,&bits,4);self.setMoney(remaining);
    building->setLevel(next,true);self.focus(nullptr,true);self.focus(building,true); // native 00495fd0 order includes shared Upgrade sound
}
void Manufacture::rightPress(void*) {auto& self=*instance();auto* building=self.movingBuilding();if(building && building->built) building->cancelMove();} // 00493bb0
void Manufacture::outsidePress(void*) {
    auto& self=*instance();auto& tips=self.tutorial();if(tips.activeStep(38) || tips.activeStep(34) || tips.activeStep(33)) return;
    const auto point=self.context_.interaction.cursor();if(!productionPointInside({10,10,790,500},point) || self.movingBuilding()) return;
    for(auto* building:self.slots_) if(building && rectangleHit(*building,point)) return; // 00493aa0 / 00417910, no button mask
    if(self.focused_) self.focused_->hideControls();self.focus(nullptr,true);
}
void Manufacture::enabledStep() {
    if(movement) return;auto& tips=tutorial();
    if(tips.canShow(31)) {tips.showCommon(31);return;}if(tips.canShow(32)) {tips.showAction(32,build_);return;}
    if(!focused_) return;
    if(tips.canShow(38)) {tips.showArrow(38,&focused_->loadedSlider());return;}
    if(tips.canShow(48)) {
        const auto input=productionKind(focused_->buildingType).inputProduct();
        if(!warehouseGoods(input)) {
            if(warehouseFree()) giveWarehouse(input,1,true,false);
            else for(unsigned product=0;product<19;++product) if(warehouseGoods(product)) {takeWarehouse(product,1,true,false);giveWarehouse(input,1,true,false);break;}
        }
        tips.showAction(48,&focused_->loadedSlider());return;
    }
    if(tips.canShow(34)) {price_->setString(WString(L"0"));upgrade_->enable();price_->enable();tips.showAction(34,upgrade_);}
    else if(tips.canShow(33)) tips.showAction(33,destroy_);
    // Native final base step is the genuine RET 004bd820.
}
// Constructor/factory/selection/input/placement/demolition are directly
// compared; full tutorial/world providers and cGame assembly remain pending.
// Source libraries never forward these calls to the reference.
}
