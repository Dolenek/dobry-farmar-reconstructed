#include "DigBuilding.h"
#include "StateView.h"
#include "TutorialManager.h"
#include "GameConfiguration.h"
#include "ResourceManager.h"
#include "XmlDocument.h"
#include "StringConversion.h"
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <string>
namespace farmar::recovered {
namespace {
DigBuilding* currentDigBuilding=nullptr;
void child(View& view,const XmlNode& node,const char* name) {XmlNode* entry=nullptr;if(node.section(String(name),entry)) view.loadXML(*entry);}
bool shown(const View& view) {return view.state().shown && view.state().parentShown;}
}
DigBuildingLifetime::DigBuildingLifetime(DigBuilding* self) {if(currentDigBuilding) throw "Object already exist.";currentDigBuilding=self;}
DigBuildingLifetime::~DigBuildingLifetime() {currentDigBuilding=nullptr;}
DigBuilding* DigBuilding::instance() noexcept {return currentDigBuilding;}
DigBuilding::DigBuilding(View* parent,DigBuildingContext& c):ActionArea(16,parent,c.area),DigBuildingLifetime(this),Upgrade(c.upgrades,14,34),digContext_(c) {
    auto& a=c.area;
    upgrade_=new ExtendedTextAnimationButton(this,a.resources,a.settings,a.strings,a.input,a.interaction,[&c]{return c.world.tutorial().active();},a.views,a.controls);
    price_=new TextMoney(this,a.resources,a.settings,a.strings,a.views,a.controls);
    loadXML(*a.resources.loadSettingsXML(String("Farm/ActionAreas/DigBuilding.xml")));upgrade_->clickEvent=&upgradeClick;refresh();
}
void DigBuilding::loadXML(const XmlNode& node) {ActionArea::loadXML(node);Upgrade::loadXML(node);child(*upgrade_,node,"UpgradeButton");child(*price_,node,"CostText");}
void DigBuilding::loadLevel(unsigned index,const XmlNode& node) {
    auto& level=levels.at(index);const auto time=node.attribute(WString(L"DigTime"));if(!time.empty()) {std::wistringstream input(time.c_str());input>>level.digTime;}
    level.appearance.loadXML(node,digContext_.area.resources);level.maskName=toNarrow(node.attribute(WString(L"texture_mask")));
    XmlNode* hover=nullptr;if(node.section(String("Hovered"),hover)) level.hovered.loadXML(*hover,digContext_.area.resources);
    XmlNode* bare=nullptr;if(!node.section(String("NoInstrument"),bare)) return;
    level.noInstrument.loadXML(*bare,digContext_.area.resources);level.noInstrumentMask=toNarrow(bare->attribute(WString(L"texture_mask")));
    if(bare->section(String("Hovered"),hover)) level.noInstrumentHovered.loadXML(*hover,digContext_.area.resources);
}
void DigBuilding::refreshAppearance() {
    const auto current=level();const auto held=digContext_.world.heldType();auto& info=levels.at(static_cast<unsigned>(current));
    const bool bare=held==2;copyAppearance(bare ? info.noInstrument:info.appearance);const auto& mask=bare ? info.noInstrumentMask:info.maskName;
    if(mask.empty()) setMaskEnabled(false);else setMask(loadClickMask(mask));markTransformDirty();
}
void DigBuilding::refresh() {
    if(level()==-1) return;refreshAppearance();
    if(level()+1>=levelCount) {price_->hide();upgrade_->hideTips();return;}
    const auto cost=digContext_.configuration.upgradeCost(itemType,static_cast<unsigned>(level()+1));price_->setString(WString(std::to_wstring(cost)));price_->show();upgrade_->show();
    if(canUpgrade(true,true)) {price_->enable();upgrade_->enable();}
    else {price_->disable();upgrade_->disable();if(!canUpgrade(false,true)) {upgrade_->hideTips();price_->hide();}}
}
unsigned DigBuilding::investment() {const auto current=level();unsigned total=0;for(int i=1;i<current;++i) total+=digContext_.configuration.upgradeCost(itemType,static_cast<unsigned>(i));return total;}
void DigBuilding::hovered() {auto* text=dynamic_cast<TextLine*>(tips().at(1));if(!text) throw std::logic_error("DigBuilding requires first tooltip TextLine");text->setString(digContext_.configuration.upgradeStrings.at(static_cast<unsigned>(level()+1)));ExtendedFieldButton::hovered();}
void DigBuilding::unhovered() {ExtendedFieldButton::unhovered();}
void DigBuilding::clicked() {auto& world=digContext_.world;if(world.cursorOverAreaType(64,nullptr) || world.cursorOverAreaType(32,nullptr)) ButtonPrototype::clicked();else ActionArea::clicked();}
void DigBuilding::enabledStep() {
    ExtendedFieldButton::enabledStep();auto& world=digContext_.world;
    if(shown(*upgrade_) && !world.farmScene().movement && shown(world.farmScene()) && world.tutorial().canShow(28)) world.tutorial().showArrow(28,upgrade_);
}
void DigBuilding::upgradeClick(ButtonPrototype*) {
    auto& building=*currentDigBuilding;if(!building.canUpgrade(true,true)) return;const auto next=unsigned(building.level())+1;
    const auto cost=building.digContext_.configuration.upgradeCost(building.itemType,next);auto& world=building.digContext_.world;
    const unsigned difference=unsigned(world.money())-cost;int value;std::memcpy(&value,&difference,4);world.setMoney(value);building.setLevel(static_cast<int>(next),true);
    if(world.heldType()==2) {const auto current=building.level();world.animateHeldDigUpgrade(world.characterPosition(),{building.geometry().x,building.geometry().y},current,false);world.refreshHeldAppearance();}
}
}
