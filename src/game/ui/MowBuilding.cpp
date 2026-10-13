#include "MowBuilding.h"
#include "StateView.h"
#include "TutorialManager.h"
#include "GameConfiguration.h"
#include "ProfileProperties.h"
#include "ResourceManager.h"
#include "XmlDocument.h"
#include "StringConversion.h"
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <string>
namespace farmar::recovered {
namespace {
MowBuilding* currentMowBuilding=nullptr;
void child(View& view,const XmlNode& node,const char* name) {XmlNode* entry=nullptr;if(node.section(String(name),entry)) view.loadXML(*entry);}
template<class T> void numeric(T& value,const XmlNode& node,const wchar_t* key) {const auto text=node.attribute(WString(key));if(!text.empty()) {std::wistringstream stream(text.c_str());stream>>value;}}
bool shown(const View& view) {return view.state().shown && view.state().parentShown;}
int signedWord(unsigned word) {int result;std::memcpy(&result,&word,4);return result;}
}
MowBuildingLifetime::MowBuildingLifetime(MowBuilding* self) {if(currentMowBuilding) throw "Object already exist.";currentMowBuilding=self;}
MowBuildingLifetime::~MowBuildingLifetime() {currentMowBuilding=nullptr;}
MowBuilding* MowBuilding::instance() noexcept {return currentMowBuilding;}
MowBuilding::MowBuilding(View* parent,MowBuildingContext& c):ActionArea(8,parent,c.area),MowBuildingLifetime(this),Upgrade(c.upgrades,11,31),mowContext_(c) {
    auto& a=c.area;upgrade_=new ExtendedTextAnimationButton(this,a.resources,a.settings,a.strings,a.input,a.interaction,[&c]{return c.world.tutorial().active();},a.views,a.controls);
    sign_=new Picture(this,a.resources,a.views,a.controls);substrate_=new Picture(sign_,a.resources,a.views,a.controls);price_=new TextMoney(this,a.resources,a.settings,a.strings,a.views,a.controls);
    loadXML(*a.resources.loadSettingsXML(String("Farm/ActionAreas/MowBuilding.xml")));upgrade_->clickEvent=&upgradeClick;refresh();
}
void MowBuilding::loadXML(const XmlNode& node) {ActionArea::loadXML(node);Upgrade::loadXML(node);child(*upgrade_,node,"UpgradeButton");child(*price_,node,"CostText");child(*sign_,node,"WotInScythSign");child(*substrate_,node,"WotSubstrate");}
void MowBuilding::loadLevel(unsigned index,const XmlNode& node) {
    auto& info=levels.at(index);numeric(info.productionNum,node,L"ProductionNum");info.maskName=toNarrow(node.attribute(WString(L"texture_mask")));numeric(info.mowTime,node,L"MowTime");info.appearance.loadXML(node,mowContext_.area.resources);
    XmlNode* hover=nullptr;if(node.section(String("Hovered"),hover)) info.hovered.loadXML(*hover,mowContext_.area.resources);XmlNode* bare=nullptr;if(!node.section(String("NoInstrument"),bare)) return;
    info.noInstrument.loadXML(*bare,mowContext_.area.resources);info.noInstrumentMask=toNarrow(bare->attribute(WString(L"texture_mask")));if(bare->section(String("Hovered"),hover)) info.noInstrumentHovered.loadXML(*hover,mowContext_.area.resources);
}
void MowBuilding::refreshAppearance() {
    auto& w=mowContext_.world;auto& info=levels.at(unsigned(level()));const bool bare=w.heldType()==6;copyAppearance(bare ? info.noInstrument:info.appearance);const auto& mask=bare ? info.noInstrumentMask:info.maskName;
    if(mask.empty()) setMaskEnabled(false);else setMask(loadClickMask(mask));
    if(!bare && level()<3 && w.heldProductCount()!=0) {
        for(unsigned i=0;i<8;++i) if(mowContext_.profile.uints["PCC"+std::to_string(i)]!=0) {sign_->copyAppearance(mowContext_.configuration.products[i].icon);break;}
        sign_->show();
    }else sign_->hide();markTransformDirty();
}
void MowBuilding::refresh() {
    if(level()==-1) return;refreshAppearance();auto& w=mowContext_.world;
    if(w.personPresent() && w.heldType()==6) {const auto count=w.heldProductCount();w.hidePersonHeldIndicator();w.updatePersonHeldIndicator(true,count,levels.at(unsigned(level())).productionNum);}
    if(level()+1>=levelCount) {price_->hide();upgrade_->hideTips();return;}
    price_->setString(WString(std::to_wstring(mowContext_.configuration.upgradeCost(itemType,unsigned(level()+1)))));price_->show();upgrade_->show();
    if(canUpgrade(true,true)) {price_->enable();upgrade_->enable();}else {price_->disable();upgrade_->disable();if(!canUpgrade(false,true)) {upgrade_->hideTips();price_->hide();}}
}
unsigned MowBuilding::investment() {const auto current=level();unsigned result=0;for(int i=1;i<current;++i) result+=mowContext_.configuration.upgradeCost(itemType,unsigned(i));return result;}
void MowBuilding::hovered() {auto* text=dynamic_cast<TextLine*>(tips().at(1));if(!text) throw std::logic_error("MowBuilding requires first tooltip TextLine");text->setString(mowContext_.configuration.upgradeStrings.at(unsigned(level()+1)));ExtendedFieldButton::hovered();}
void MowBuilding::unhovered() {ExtendedFieldButton::unhovered();}
void MowBuilding::clicked() {auto& w=mowContext_.world;if(w.cursorOverAreaType(64,nullptr) || w.cursorOverAreaType(32,nullptr)) ButtonPrototype::clicked();else ActionArea::clicked();}
void MowBuilding::enabledStep() {
    ExtendedFieldButton::enabledStep();auto& w=mowContext_.world;if(shown(*upgrade_) && !w.farmScene().movement && shown(w.farmScene())) {
        if(level()==0) {if(w.tutorial().canShow(27)) w.tutorial().showArrow(27,upgrade_);}else if(level()==2 && w.tutorial().canShow(47)) w.tutorial().showArrow(47,upgrade_);
    }
}
void MowBuilding::upgradeClick(ButtonPrototype*) {
    auto& b=*currentMowBuilding;if(!b.canUpgrade(true,true)) return;auto& w=b.mowContext_.world;const auto next=unsigned(b.level())+1;
    const auto cost=b.mowContext_.configuration.upgradeCost(b.itemType,next);w.setMoney(signedWord(unsigned(w.money())-cost));b.setLevel(int(next),true);
    if(w.heldType()==6) {w.animateHeldUpgrade(w.characterPosition(),{b.geometry().x,b.geometry().y},b.level()+4,false);w.refreshHeldAppearance();}
}
}
