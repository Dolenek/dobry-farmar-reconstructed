#include "SeedPackage.h"
#include "FarmField.h"
#include "StateView.h"
#include "WaterSource.h"
#include "TutorialManager.h"
#include "ResourceManager.h"
#include "Resource.h"
#include "XmlDocument.h"
#include "StringConversion.h"
#include "OriginalArithmetic.h"
#include <cstring>
#include <sstream>
#include <stdexcept>
namespace farmar::recovered {
namespace {
bool shown(const View& v) {return v.state().shown && v.state().parentShown;}
int signedWord(unsigned value) {int result;std::memcpy(&result,&value,4);return result;}
void refreshWater() {auto* water=WaterSource::instance();if(!water) throw std::logic_error("SeedPackage requires actual WaterSource");water->refreshSprinkle();}
}
SeedPackage::SeedPackage(View* parent,unsigned kind,unsigned quantity,const Point2& position,unsigned mode,bool owed,SeedPackageContext& c)
    :ActionArea(32,parent,c.area),unpaid(owed),seedContext_(c) {
    use_=new ProgressDiscrete(this,c.area.resources,c.area.views,c.area.controls);
    loadXML(*c.area.resources.loadSettingsXML(String("Farm/ActionAreas/SeedPackage.xml")));
    kind_=kind;count_=quantity;if(use_->current!=count_) {use_->current=count_;use_->refresh();}FieldObject::setPosition(position);
    if(mode==2) {c.world.setSeedMoveActive(true);c.world.setSelectedSeed(this);}setStatus(mode);
    copyAppearance(c.configuration.seeds.at(kind).icon);markTransformDirty();
}
SeedPackage::~SeedPackage() {if(takeSound_) takeSound_->releaseReference();if(dropSound_) dropSound_->releaseReference();}
void SeedPackage::loadXML(const XmlNode& node) {
    ActionArea::loadXML(node);normalColor=geometry().color;
    const auto color=node.attribute(WString(L"UnfitColor"));if(!color.empty()) {unsigned value=0;std::wistringstream stream(color.c_str());stream>>std::hex>>value;unfitColor=Color::fromARGB(value);}
    XmlNode* definition=nullptr;if(node.section(String("Use"),definition)) use_->loadXML(*definition);
    const auto sound=[&](const wchar_t* key,cResource*& value) {const auto path=toNarrow(node.attribute(WString(key)));if(!path.empty()) {if(!seedContext_.area.interaction.loadSound) throw std::logic_error("SeedPackage requires original sound loader");value=seedContext_.area.interaction.loadSound(path);}};
    // Like the original XML routine, reload retains old acquired sound refs.
    sound(L"TakeSound",takeSound_);sound(L"DropSound",dropSound_);
}
void SeedPackage::setCount(unsigned value) {if(count_!=value) count_=value;if(use_->current!=count_) {use_->current=count_;use_->refresh();}}
void SeedPackage::setStatus(unsigned value) {
    auto& w=seedContext_.world;if(status_!=value) {status_=value;w.setFarmActionButtonEnabled(value!=2);
        if(status_==0) {if(pointInRect(seedContext_.area.interaction.cursor())) use_->show();else use_->hide();}}
    if(status_==3) {use_->hide();show();}if(value==3) {use_->hide();hideTips();}refreshWater();
}
Point2 SeedPackage::dragPosition(const Point2& cursor) const {return {float(originalSubtract(cursor[0],dragOffset[0])),float(originalSubtract(cursor[1],dragOffset[1]))};}
void SeedPackage::play(cResource* value) {if(value) seedContext_.area.interaction.playClick(value);}
void SeedPackage::enabledStep() {
    ExtendedFieldButton::enabledStep();actions_.step();auto& c=seedContext_;auto& w=c.world;
    if(status_==1) {const auto cursor=c.area.interaction.cursor();const auto dx=originalSubtract(cursor[0],pressPosition[0]),dy=originalSubtract(cursor[1],pressPosition[1]);
        if(!(originalAdd(originalMultiply(dx,dx),originalMultiply(dy,dy))>=16) || w.seedMoveActive()) return;
        w.setSeedMoveActive(true);w.setSelectedSeed(this);setStatus(2);play(takeSound_);return;}
    auto& farm=w.farmScene();auto& tutorial=w.tutorial();
    if(status_==2) {tipsDirection=1;const auto point=dragPosition(c.area.interaction.cursor());setPosition(point);setColor(w.canPlaceSeed(*this,point) ? normalColor:unfitColor);
        if(farm.movement==0 && shown(farm) && tutorial.canShow(16)) tutorial.showAction(16,nullptr);return;}
    if(status_!=0) return;
    if(c.profile.uints["PCarry"]==2) {if(tutorial.canShow(21) && farm.movement==0 && shown(farm)) tutorial.showAction(21,w.fieldForSeedTutorial());}
    else if(farm.movement==0 && shown(farm) && tutorial.canShow(20)) tutorial.showAction(20,&w.digBuilding());
}
void SeedPackage::clicked() {
    ButtonPrototype::clicked();tipsDirection=1;auto& c=seedContext_;auto& w=c.world;const auto cursor=c.area.interaction.cursor();
    if(status_==2) {const auto point=dragPosition(cursor);
        if(w.canPlaceSeed(*this,point)) {ButtonPrototype::unclicked();FieldObject::setPosition(point);w.setSeedMoveActive(false);w.setSelectedSeed(nullptr);setStatus(0);play(dropSound_);
            if(unpaid) w.setMoney(signedWord(unsigned(w.money())-c.configuration.seeds.at(kind_).price));unpaid=false;}
        else if(unpaid && w.farmScene().movement==0 && shown(w.farmScene()) && !w.tutorial().activeStep(16)) {
            unpaid=false;if(w.selectedSeed()==this) {w.setSeedMoveActive(false);w.setSelectedSeed(nullptr);}setStatus(0);w.removeSeed(*this);}
    }else {auto* area=w.topAt(cursor,127);if(!area) throw std::logic_error("SeedPackage click requires original topmost area");
        if(area==this || (area->areaType==32 && static_cast<SeedPackage*>(area)->status()==3)) {setStatus(1);pressPosition=cursor;dragOffset={float(originalSubtract(cursor[0],geometry().x)),float(originalSubtract(cursor[1],geometry().y))};}}
}
void SeedPackage::unclicked() {
    const auto previous=status_;auto& c=seedContext_;auto& w=c.world;const auto cursor=c.area.interaction.cursor();tipsDirection=1;
    if(previous==2) {const auto point=dragPosition(cursor);if(w.canPlaceSeed(*this,point)) {ButtonPrototype::unclicked();FieldObject::setPosition(point);w.setSeedMoveActive(false);w.setSelectedSeed(nullptr);setStatus(0);play(dropSound_);}}
    else if(previous==0) ButtonPrototype::unclicked();
    else if(previous==1) {w.setSeedMoveActive(false);w.setSelectedSeed(nullptr);setStatus(0);ButtonPrototype::unclicked();tipsDirection=1;
        if(clickIndex<clickOffsets.size()) {const auto& delta=clickOffsets[clickIndex];const Point2 point{float(originalAdd(geometry().x,delta[0])),float(originalAdd(geometry().y,delta[1]))};++clickIndex;w.queueAreaAction(*this,point);}}
}
void SeedPackage::hovered() {
    use_->show();auto& c=seedContext_;const auto cursor=c.area.interaction.cursor();
    if(c.world.topAt(cursor,97)==this) {for(auto* area:c.world.areasAt(cursor,96)) if(area!=this) area->tipsDirection=1;ExtendedFieldButton::hovered();}
    else ButtonPrototype::hovered();
}
void SeedPackage::unhovered() {use_->hide();ExtendedFieldButton::unhovered();}
SeedPackage* createFarmSeed(unsigned kind,SeedPackageContext& c) {return new SeedPackage(&c.world.farmScene().movingPicture(),kind,4,c.area.interaction.cursor(),2,true,c);}
}
