#include "Sprinkle.h"
#include "WaterSource.h"
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
void refreshWaterSource() {if(auto* water=WaterSource::instance()) water->refreshSprinkle();}
int signedWord(unsigned word) {int value;std::memcpy(&value,&word,4);return value;}
}
std::string Sprinkle::key(const char* prefix) const {return std::string(prefix)+std::to_string(id);}
Sprinkle::Sprinkle(View* parent,unsigned mode,unsigned index,SprinkleContext& c):ActionArea(64,parent,c.area),id(index),sprinkleContext_(c) {
    loadXML(*c.area.resources.loadSettingsXML(String("Farm/ActionAreas/Sprinkle.xml")));
    setStoredPosition({c.profile.floats[key("SPXPos")],c.profile.floats[key("SPYPos")]});
    effect_=new SprinkleEmitter(this,c.area.resources,c.random,String("Effects/SprinkleEffect.xml"),c.area.views,c.area.controls);
    effect_->start(0);positionEffect();
    if(mode==2) c.world.setSprinkleMoveActive(true);
    setStatus(mode);installed=c.profile.booleans[key("SPExist")];
}
Sprinkle::~Sprinkle() {
    if(installSound_) installSound_->releaseReference();
    if(dropSound_) dropSound_->releaseReference();
    if(takeSound_) takeSound_->releaseReference();
}
void Sprinkle::loadXML(const XmlNode& node) {
    ActionArea::loadXML(node);normalColor=geometry().color;
    const auto r=node.attribute(WString(L"Radius"));if(!r.empty()) {std::wistringstream input(r.c_str());input>>radius;}
    const auto color=node.attribute(WString(L"UnfitColor"));if(!color.empty()) {unsigned value=0;std::wistringstream input(color.c_str());input>>std::hex>>value;unfitColor=Color::fromARGB(value);}
    auto sound=[&](const wchar_t* name,cResource*& target) {const auto path=toNarrow(node.attribute(WString(name)));if(!path.empty()) {if(!sprinkleContext_.area.interaction.loadSound) throw std::logic_error("Sprinkle requires original sound loader");target=sprinkleContext_.area.interaction.loadSound(path);}};
    // Reload intentionally retains the old acquired references, as the native body.
    sound(L"DropSound",dropSound_);sound(L"InstallSound",installSound_);sound(L"TakeSound",takeSound_);
}
void Sprinkle::setStoredPosition(const Point2& point) {FieldObject::setPosition(point);auto& p=sprinkleContext_.profile;p.floats[key("SPXPos")]=point[0];p.floats[key("SPYPos")]=point[1];}
unsigned Sprinkle::status() {return sprinkleContext_.profile.uints[key("SPStatus")];}
Point2 Sprinkle::dragPosition(const Point2& cursor) const {return {float(originalSubtract(cursor[0],dragOffset[0])),float(originalSubtract(cursor[1],dragOffset[1]))};}
void Sprinkle::positionEffect() {
    auto& p=sprinkleContext_.profile;effect_->setDeep(float(originalAdd(deep(),1)));
    effect_->setPosition({p.floats[key("SPXPos")],float(originalSubtract(p.floats[key("SPYPos")],10))});
}
void Sprinkle::setStatus(unsigned value) {
    auto& p=sprinkleContext_.profile;auto& w=sprinkleContext_.world;if(p.uints[key("SPStatus")]!=value) p.uints[key("SPStatus")]=value;
    if(value==0) {fields_=w.wateredFields({geometry().x,geometry().y},radius);w.clearWaterPreview();}
    else if(value==2) {w.setFarmActionButtonEnabled(false);effect_->emission.generating=false;refreshWaterSource();return;}
    w.setFarmActionButtonEnabled(true);positionEffect();if(value==0) effect_->emission.generating=!fields_.empty();refreshWaterSource();
}
void Sprinkle::play(cResource* sound) {if(sound) {if(!sprinkleContext_.area.interaction.playClick) throw std::logic_error("Sprinkle requires original click sound player");sprinkleContext_.area.interaction.playClick(sound);}}
void Sprinkle::clicked() {
    ButtonPrototype::clicked();tipsDirection=1;auto& w=sprinkleContext_.world;const auto cursor=sprinkleContext_.area.interaction.cursor();
    if(status()!=2) {if(w.areaAt(cursor,127)!=this) return;setStatus(1);pressPosition=sprinkleContext_.area.interaction.cursor();
        dragOffset={float(originalSubtract(pressPosition[0],geometry().x)),float(originalSubtract(pressPosition[1],geometry().y))};return;}
    if(w.canPlaceSprinkle(*this,cursor)) {
        setStoredPosition(dragPosition(cursor));w.setSprinkleMoveActive(false);setStatus(0);
        if(installed) {play(dropSound_);return;}
        auto* water=WaterSource::instance();if(!water) throw std::logic_error("Sprinkle purchase requires actual WaterSource");
        const auto money=signedWord(unsigned(w.money())-water->sprinkleCost);
        if(money>=0) {installed=true;sprinkleContext_.profile.booleans[key("SPExist")]=true;play(installSound_);w.setMoney(money);return;}
    } else {if(installed) return;sprinkleContext_.profile.booleans[key("SPExist")]=false;}
    w.setSprinkleMoveActive(false);setStatus(0);w.removeSprinkle(*this);refreshWaterSource();
}
void Sprinkle::unclicked() {
    ButtonPrototype::unclicked();tipsDirection=1;auto& w=sprinkleContext_.world;
    if(status()==2) {const auto cursor=sprinkleContext_.area.interaction.cursor();if(w.canPlaceSprinkle(*this,cursor)) {setStoredPosition(dragPosition(cursor));w.setSprinkleMoveActive(false);setStatus(0);
            if(!installed) {installed=true;sprinkleContext_.profile.booleans[key("SPExist")]=true;play(installSound_);}else play(dropSound_);}}
    else {w.setSprinkleMoveActive(false);setStatus(0);}
    // Actual release installs without the clicked branch's Money deduction.
}
void Sprinkle::hovered() {
    auto& w=sprinkleContext_.world;const auto cursor=sprinkleContext_.area.interaction.cursor();
    if(w.buildingAt(cursor) || w.areaAt(cursor,96)!=this) {ButtonPrototype::hovered();return;}
    for(auto* area:w.areasAt(cursor,96)) if(area!=this) area->tipsDirection=1;
    ExtendedFieldButton::hovered();
}
void Sprinkle::unhovered() {ExtendedFieldButton::unhovered();}
void Sprinkle::enabledStep() {
    ExtendedFieldButton::enabledStep();auto& w=sprinkleContext_.world;
    if(status()==1) {const auto point=sprinkleContext_.area.interaction.cursor();const auto dx=originalSubtract(point[0],pressPosition[0]),dy=originalSubtract(point[1],pressPosition[1]);
        const auto squared=originalAdd(originalMultiply(dx,dx),originalMultiply(dy,dy));if(squared>=16 && !w.sprinkleMoveActive()) {w.setSprinkleMoveActive(true);setStatus(2);play(takeSound_);}}
    else if(status()==2) {tipsDirection=1;const auto point=dragPosition(sprinkleContext_.area.interaction.cursor());setPosition(point);effect_->setPosition({point[0],float(originalSubtract(point[1],10))});
        if(w.canPlaceSprinkle(*this,point)) {setColor(normalColor);w.clearWaterPreview();for(auto* field:w.wateredFields({geometry().x,geometry().y},radius)) w.previewWater(*field,true);}
        else {setColor(unfitColor);w.clearWaterPreview();}}
    else {if(!uiElapsedTime()) throw std::logic_error("Sprinkle requires original elapsed timer");auto* water=WaterSource::instance();if(!water) throw std::logic_error("Sprinkle watering requires actual WaterSource");
        const float elapsed=uiElapsedTime()();const float amount=float(originalMultiply(water->capacity(),elapsed));for(auto* field:fields_) w.addWater(*field,amount);}
}
Sprinkle* createFarmSprinkle(View* parent,unsigned cost,unsigned& count,SprinkleContext& c) {
    if(c.world.money()<signedWord(cost) || count>=3) return nullptr;
    unsigned index=0;for(;index<3;++index) if(!c.profile.booleans[std::string("SPExist")+std::to_string(index)]) break;
    auto* result=new Sprinkle(parent,2,index,c);++count;
    result->setPosition(c.area.interaction.cursor()); // original virtual c0: no SPXPos/SPYPos write
    return result;
}
}
