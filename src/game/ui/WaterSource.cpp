#include "WaterSource.h"
#include "StateView.h"
#include "TutorialManager.h"
#include "GameConfiguration.h"
#include "ResourceManager.h"
#include "XmlDocument.h"
#include "StringConversion.h"
#include "OriginalArithmetic.h"
#include <algorithm>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <string>
namespace farmar::recovered {
namespace {
WaterSource* currentWaterSource=nullptr;
void child(View& view,const XmlNode& node,const char* name) {XmlNode* entry=nullptr;if(node.section(String(name),entry)) view.loadXML(*entry);}
template<class T> void numeric(T& value,const XmlNode& node,const wchar_t* key) {const auto text=node.attribute(WString(key));if(!text.empty()) {std::wistringstream stream(text.c_str());stream>>value;}}
bool shown(const View& view) {return view.state().shown && view.state().parentShown;}
int signedWord(unsigned word) {int result;std::memcpy(&result,&word,4);return result;}
bool rectangleHit(const Control& control,const Point2& point) {
    const auto& g=control.geometry();const float left=float(originalSubtract(g.x,originalMultiply(g.width,0.5))),top=float(originalSubtract(g.y,originalMultiply(g.height,0.5)));
    const auto right=originalAdd(left,g.width);const float bottom=float(originalAdd(top,g.height));return left<=point[0] && point[0]<=right && top<=point[1] && point[1]<=bottom;
}
}
WaterSourceLifetime::WaterSourceLifetime(WaterSource* self) {if(currentWaterSource) throw "Object already exist.";currentWaterSource=self;}
WaterSourceLifetime::~WaterSourceLifetime() {currentWaterSource=nullptr;}
WaterSource* WaterSource::instance() noexcept {return currentWaterSource;}
WaterSource::WaterSource(View* parent,WaterSourceContext& c):ActionArea(4,parent,c.area),WaterSourceLifetime(this),Upgrade(c.upgrades,13,33),waterContext_(c) {
    auto& a=c.area;
    upgrade_=new ExtendedTextAnimationButton(this,a.resources,a.settings,a.strings,a.input,a.interaction,[&c]{return c.world.tutorial().active();},a.views,a.controls);
    sprinkle_=new ExtendedTextAnimationButton(this,a.resources,a.settings,a.strings,a.input,a.interaction,[&c]{return c.world.tutorial().active();},a.views,a.controls);
    price_=new TextMoney(this,a.resources,a.settings,a.strings,a.views,a.controls);sprinklePrice_=new TextMoney(this,a.resources,a.settings,a.strings,a.views,a.controls);
    loadXML(*a.resources.loadSettingsXML(String("Farm/ActionAreas/WaterSource.xml")));sprinklePrice_->setString(WString(std::to_wstring(sprinkleCost)));
    upgrade_->clickEvent=&upgradeClick;sprinkle_->clickEvent=&sprinkleClick;refresh();
}
void WaterSource::loadXML(const XmlNode& node) {
    ActionArea::loadXML(node);Upgrade::loadXML(node);numeric(sprinkleCost,node,L"SprinkleCost");child(*upgrade_,node,"UpgradeButton");child(*sprinkle_,node,"SprinkleButton");child(*price_,node,"CostText");child(*sprinklePrice_,node,"SprinkleCostText");
    sprinkleNormalDepth=sprinkle_->deep();numeric(sprinkleDepthWhenMoving,node,L"SprinkleButtonDeepThanGet");XmlNode* states=nullptr;
    if(node.section(String("SprinkleButtonStates"),states)) for(unsigned i=0;i<(std::min<std::size_t>)(3,states->children().size());++i) {XmlNode* state=nullptr;if(states->section(i,state)) sprinkleStates[i].loadXML(*state,waterContext_.area.resources);}
}
void WaterSource::loadLevel(unsigned index,const XmlNode& node) {
    auto& info=levels.at(index);numeric(info.capacity,node,L"WaterCapacity");info.maskName=toNarrow(node.attribute(WString(L"texture_mask")));info.appearance.loadXML(node,waterContext_.area.resources);
    XmlNode* hover=nullptr;if(node.section(String("Hovered"),hover)) info.hovered.loadXML(*hover,waterContext_.area.resources);XmlNode* bare=nullptr;if(!node.section(String("NoInstrument"),bare)) return;
    info.noInstrument.loadXML(*bare,waterContext_.area.resources);info.noInstrumentMask=toNarrow(bare->attribute(WString(L"texture_mask")));if(bare->section(String("Hovered"),hover)) info.noInstrumentHovered.loadXML(*hover,waterContext_.area.resources);
}
void WaterSource::refreshAppearance() {
    auto& info=levels.at(unsigned(level()));const bool bare=waterContext_.world.heldType()==2;copyAppearance(bare ? info.noInstrument:info.appearance);const auto& mask=bare ? info.noInstrumentMask:info.maskName;
    if(mask.empty()) setMaskEnabled(false);else setMask(loadClickMask(mask));markTransformDirty();
}
void WaterSource::refreshSprinkle() {
    if(level()!=2) return;auto& w=waterContext_.world;const auto count=w.sprinkleCount();
    if(count<3) {sprinkle_->copyAppearance(sprinkleStates[count]);sprinkle_->show();sprinklePrice_->show();}else {sprinkle_->hideTips();sprinklePrice_->hide();}
    sprinkle_->setDeep(w.sprinkleMoveActive() ? sprinkleDepthWhenMoving:sprinkleNormalDepth);
    if(!w.seedMoveActive() && !w.sprinkleMoveActive() && w.money()>=signedWord(sprinkleCost)) {sprinkle_->enable();sprinklePrice_->enable();}else {sprinkle_->disable();sprinklePrice_->disable();}
}
void WaterSource::refresh() {
    const auto current=level();if(current!=-1) {
        refreshAppearance();if(level()+1>=levelCount) {price_->hide();upgrade_->hideTips();}else {
            price_->setString(WString(std::to_wstring(waterContext_.configuration.upgradeCost(itemType,unsigned(level()+1)))));price_->show();upgrade_->show();
            if(canUpgrade(true,true)) {price_->enable();upgrade_->enable();}else {price_->disable();upgrade_->disable();if(!canUpgrade(false,true)) {upgrade_->hideTips();price_->hide();}}
        }
        auto& w=waterContext_.world;if(current!=0 && w.heldType()==1) {w.setHeldType(0);w.updatePersonAppearance();w.cancelPersonAction(String("d:\\work\\farmer\\src\\farmer\\game\\farm\\Person.h157"));}
    }refreshSprinkle();
}
double WaterSource::capacity() {const auto value=level();return value==-1 ? 0.:double(levels.at(unsigned(value)).capacity);}
unsigned WaterSource::investment() {const auto value=level();unsigned result=0;for(int i=1;i<value;++i) result+=waterContext_.configuration.upgradeCost(itemType,unsigned(i));return result;}
void WaterSource::hovered() {auto* text=dynamic_cast<TextLine*>(tips().at(1));if(!text) throw std::logic_error("WaterSource requires first tooltip TextLine");text->setString(waterContext_.configuration.upgradeStrings.at(unsigned(level()+1)));ExtendedFieldButton::hovered();}
void WaterSource::unhovered() {ExtendedFieldButton::unhovered();}
void WaterSource::clicked() {auto& w=waterContext_.world;const bool overUpgrade=upgrade_->state().shown && rectangleHit(*upgrade_,waterContext_.area.interaction.cursor());if(overUpgrade || w.cursorOverAreaType(64,nullptr) || w.cursorOverAreaType(32,nullptr)) ButtonPrototype::clicked();else ActionArea::clicked();}
void WaterSource::unclicked() {if(level()==2) {ButtonPrototype::unclicked();return;}ButtonPrototype::unclicked();tipsDirection=1;}
void WaterSource::enabledStep() {
    ExtendedFieldButton::enabledStep();auto& w=waterContext_.world;if(shown(*upgrade_) && !w.farmScene().movement && shown(w.farmScene())) {if(level()==0) {if(w.tutorial().canShow(25)) w.tutorial().showArrow(25,upgrade_);}else if(level()==1 && w.tutorial().canShow(46)) w.tutorial().showArrow(46,upgrade_);}
    if(shown(*sprinkle_) && !w.farmScene().movement && shown(w.farmScene()) && w.tutorial().canShow(29)) w.tutorial().showArrow(29,sprinkle_);
}
void WaterSource::sprinkleClick(ButtonPrototype*) {auto& b=*currentWaterSource;if(auto* created=b.waterContext_.world.createSprinkle(b.sprinkleCost)) created->setState(ButtonState::Pressed);}
void WaterSource::upgradeClick(ButtonPrototype*) {
    auto& b=*currentWaterSource;if(!b.canUpgrade(true,true)) return;auto& w=b.waterContext_.world;const auto held=w.heldType();const auto next=unsigned(b.level())+1;
    const auto cost=b.waterContext_.configuration.upgradeCost(b.itemType,next);w.setMoney(signedWord(unsigned(w.money())-cost));b.setLevel(int(next),true);
    if(held==1) w.animateHeldUpgrade(w.characterPosition(),{b.geometry().x,b.geometry().y},0,false);
}
}
