#include "Warehouse.h"
#include "Manufacture.h"
#include "GameMenu.h"
#include "TutorialManager.h"
#include "GameConfiguration.h"
#include "ProfileProperties.h"
#include "ResourceManager.h"
#include "XmlDocument.h"
#include "OriginalArithmetic.h"
#include "StringConversion.h"
#include <algorithm>
#include <cstring>
#include <sstream>
#include <string>
namespace farmar::recovered {
namespace {
Warehouse* warehouseSingleton=nullptr;
std::string goodsKey(unsigned product) {return "WGoodCount"+std::to_string(product);}
bool shown(const View& v) {return v.state().shown && v.state().parentShown;}
void child(View& v,const XmlNode& node,const char* name) {XmlNode* section=nullptr;if(node.section(String(name),section)) v.loadXML(*section);}
int signedWord(unsigned value) {int result;std::memcpy(&result,&value,4);return result;}
}
Warehouse* Warehouse::instance() noexcept {return warehouseSingleton;}
WarehouseLifetime::WarehouseLifetime(Warehouse* self) {if(warehouseSingleton) throw "Object already exist.";warehouseSingleton=self;}
WarehouseLifetime::~WarehouseLifetime() {warehouseSingleton=nullptr;}
Warehouse::Warehouse(View* parent,WarehouseContext& c)
    :ActionArea(2,parent,c.area),WarehouseLifetime(this),Upgrade(c.upgrades,12,32),warehouseContext_(c) {
    auto& a=c.area;
    upgrade_=new ExtendedTextAnimationButton(this,a.resources,a.settings,a.strings,a.input,a.interaction,[&c]{return c.world.tutorial().active();},a.views,a.controls);
    price_=new TextMoney(this,a.resources,a.settings,a.strings,a.views,a.controls);
    substrate_=new Picture(this,a.resources,a.views,a.controls);
    loading_=new ProgressBar(this,a.resources,a.views,a.controls);
    loadXML(*a.resources.loadSettingsXML(String("Farm/ActionAreas/Warehouse.xml")));
    upgrade_->clickEvent=&upgradeClick;refresh();
    for(unsigned i=0;i<19;++i) occupied+=goods(i);
    // Original stores the bottom-center anchor before adding child offsets.
    const float x=geometry().x;
    const float y=float(originalAdd(originalMultiply(geometry().height,0.5),geometry().y));
    for(Control* control:{static_cast<Control*>(substrate_),static_cast<Control*>(loading_)})
        control->setPosition({float(originalAdd(x,control->geometry().x)),float(originalAdd(y,control->geometry().y))});
    updateLoading();c.world.hud().refreshWarehouse();
}
Warehouse::~Warehouse()=default; // levels -> Upgrade -> singleton -> Field tooltip ownership
void Warehouse::loadXML(const XmlNode& node) {
    ActionArea::loadXML(node);Upgrade::loadXML(node);
    child(*substrate_,node,"LoadingSubstrain");child(*loading_,node,"Loading");
    child(*upgrade_,node,"UpgradeButton");child(*price_,node,"CostText");updateLoading();
}
void Warehouse::loadLevel(unsigned index,const XmlNode& node) {
    auto& record=levels.at(index);
    const auto count=node.attribute(WString(L"ProductionNum"));if(!count.empty()) {std::wistringstream stream(count.c_str());stream>>record.capacity;}
    record.maskName=toNarrow(node.attribute(WString(L"texture_mask")));
    record.appearance.loadXML(node,warehouseContext_.area.resources);
    XmlNode* hover=nullptr;if(node.section(String("Hovered"),hover)) record.hovered.loadXML(*hover,warehouseContext_.area.resources);
    // 00467910 does not copy primary appearance into hovered on a missing child.
}
void Warehouse::refresh() {
    const int current=level();if(current==-1) return;
    auto& record=levels.at(static_cast<unsigned>(current));copyAppearance(record.appearance);
    if(record.maskName.empty()) setMaskEnabled(false);else setMask(loadClickMask(record.maskName));
    markTransformDirty();updateLoading();warehouseContext_.world.hud().refreshWarehouse();
    if(level()+1>=levelCount) {price_->hide();upgrade_->hideTips();return;}
    const auto cost=warehouseContext_.configuration.upgradeCost(itemType,static_cast<unsigned>(level()+1));
    price_->setString(WString(std::to_wstring(cost)));price_->show();upgrade_->show();
    if(canUpgrade(true,true)) {price_->enable();upgrade_->enable();}
    else {price_->disable();upgrade_->disable();if(!canUpgrade(false,true)) {upgrade_->hideTips();price_->hide();}}
}
unsigned Warehouse::goods(unsigned product) {return product<19 ? warehouseContext_.profile.uints[goodsKey(product)]:0;}
unsigned Warehouse::freeSpace() {return levels.at(static_cast<unsigned>(level())).capacity-occupied;}
void Warehouse::updateLoading() {
    const auto capacity=levels.at(static_cast<unsigned>(level())).capacity;
    loading_->current=float(originalDivide(originalMultiply(originalUnsigned(occupied),100),originalUnsigned(capacity)));
    loading_->updateLid();
}
void Warehouse::stockChanged(bool market,bool preview) {
    auto& world=warehouseContext_.world;updateLoading();world.manufactureScene().refreshHouseControls();world.hud().refreshWarehouse();
    if(market) world.refreshMarketStocks();world.hud().stockChanged(preview);world.refreshWarehouseTip();
}
unsigned Warehouse::give(unsigned product,unsigned count,bool market,bool preview) {
    if(product>=19) return 0;count=(std::min)(count,freeSpace());if(!count) return 0;
    const auto value=warehouseContext_.profile.uints[goodsKey(product)];warehouseContext_.profile.uints[goodsKey(product)]=value+count;
    occupied+=count;stockChanged(market,preview);return count;
}
unsigned Warehouse::take(unsigned product,unsigned count,bool market,bool preview) {
    if(product>=19) return 0;const auto value=warehouseContext_.profile.uints[goodsKey(product)];count=(std::min)(value,count);if(!count) return 0;
    occupied-=count;warehouseContext_.profile.uints[goodsKey(product)]=value-count;stockChanged(market,preview);return count;
}
unsigned Warehouse::investment() {
    const int current=level();unsigned total=0;for(int i=1;i<current;++i) total+=warehouseContext_.configuration.upgradeCost(itemType,static_cast<unsigned>(i));return total;
}
void Warehouse::hovered() {
    auto* text=dynamic_cast<TextLine*>(tips().at(1));if(!text) throw std::logic_error("Warehouse requires first tooltip TextLine");
    text->setString(warehouseContext_.configuration.upgradeStrings.at(static_cast<unsigned>(level()+1)));ExtendedFieldButton::hovered();
}
void Warehouse::unhovered() {ExtendedFieldButton::unhovered();}
void Warehouse::clicked() {
    auto& world=warehouseContext_.world;
    if(world.cursorOverAreaType(64,nullptr) || world.cursorOverAreaType(32,nullptr)) ButtonPrototype::clicked();
    else ActionArea::clicked();
}
void Warehouse::enabledStep() {
    ExtendedFieldButton::enabledStep();auto& world=warehouseContext_.world;
    if(shown(*upgrade_) && !world.farmScene().movement && shown(world.farmScene()) && world.tutorial().canShow(26)) world.tutorial().showArrow(26,upgrade_);
}
void Warehouse::upgradeClick(ButtonPrototype*) {
    auto& warehouse=*warehouseSingleton;if(!warehouse.canUpgrade(true,true)) return;
    const auto next=static_cast<unsigned>(warehouse.level())+1U;
    const auto cost=warehouse.warehouseContext_.configuration.upgradeCost(warehouse.itemType,next);
    auto& world=warehouse.warehouseContext_.world;world.setMoney(signedWord(static_cast<unsigned>(world.money())-cost));
    warehouse.setLevel(signedWord(next),true);
}
}
