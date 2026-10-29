#include "FarmField.h"
#include "FieldTip.h"
#include "GameConfiguration.h"
#include "GameRandom.h"
#include "StateView.h"
#include "TutorialManager.h"
#include "ResourceManager.h"
#include "Resource.h"
#include "XmlDocument.h"
#include "OriginalArithmetic.h"
#include "StringConversion.h"
#include <algorithm>
#include <cstring>
#include <sstream>
#include <stdexcept>
namespace farmar::recovered {
namespace {
using HiddenPictureStorage=std::shared_ptr<std::pmr::monotonic_buffer_resource>;
class FieldHiddenPicture final:public Picture {
public:
    using Picture::Picture;
    static void* operator new(std::size_t size,const HiddenPictureStorage& storage) {
        static_assert(sizeof(HiddenPictureStorage)%alignof(FieldHiddenPicture)==0,"Overlay allocation alignment");
        auto* header=static_cast<HiddenPictureStorage*>(storage->allocate(sizeof(HiddenPictureStorage)+size,alignof(FieldHiddenPicture)));
        ::new(static_cast<void*>(header)) HiddenPictureStorage(storage);
        return header+1;
    }
    static void operator delete(void* pointer) noexcept {
        auto* header=static_cast<HiddenPictureStorage*>(pointer)-1;
        auto keepAlive=std::move(*header);
        header->~HiddenPictureStorage();
        // Slot memory is released with the final context/overlay reference.
        // Keep the resource alive until after its own allocation header dies.
    }
    static void operator delete(void* pointer,const HiddenPictureStorage&) noexcept {operator delete(pointer);}
};

int signedWord(unsigned value) {int result;std::memcpy(&result,&value,4);return result;}
bool shown(const View& v) {return v.state().shown && v.state().parentShown;}
template<class T> void number(T& value,const XmlNode& node,const wchar_t* name) {const auto text=node.attribute(WString(name));if(!text.empty()) {std::wistringstream stream(text.c_str());stream>>value;}}
void pair(Point2& value,const XmlNode& node,const wchar_t* name) {const auto text=node.attribute(WString(name));if(!text.empty()) {std::wistringstream stream(text.c_str());stream>>value[0];stream.ignore(1);stream>>value[1];}}
void child(View& view,const XmlNode& node,const char* name) {XmlNode* part=nullptr;if(node.section(String(name),part)) view.loadXML(*part);}
float elapsed() {if(!uiElapsedTime()) throw std::logic_error("FarmField requires original elapsed timer");return uiElapsedTime()();}
}
std::string FarmField::key(const char* prefix) const {return std::string(prefix)+std::to_string(id);}
FarmField::FarmField(View* parent,unsigned group,unsigned index,int upgrade,FarmFieldContext& c):ActionArea(1,parent,c.area),id(group*17+index),upgradeIndex(upgrade),fieldContext_(c) {
    auto& a=c.area;waterSign_=new Picture(this,a.resources,a.views,a.controls);readySign_=new Picture(this,a.resources,a.views,a.controls);
    fertilSign_=new Picture(this,a.resources,a.views,a.controls);hidden_=new(c.hiddenPictures) FieldHiddenPicture(this,a.resources,a.views,a.controls);
    loadXML(*a.resources.loadSettingsXML(String("Farm/ActionAreas/FarmField.xml")));
    const auto available=c.profile.ints["fua"],current=c.profile.ints["fcu"];refreshPurchase(current,available);
}
FarmField::~FarmField() {if(deathSound_) deathSound_->releaseReference();if(readySound_) readySound_->releaseReference();}
void FarmField::loadXML(const XmlNode& node) {
    ActionArea::loadXML(node);child(*waterSign_,node,"WaterSignPicture");child(*readySign_,node,"ReadySignPicture");child(*fertilSign_,node,"FertilSignPicture");child(*hidden_,node,"HidePic");
    number(digCost,node,L"DigCost");number(waterCapacity,node,L"water_capacity");number(fertilCapacity,node,L"fertil_capacity");number(waterSignLevel,node,L"water_sign_level");number(fertilSignLevel,node,L"fertil_sign_level");number(ploughedToGrass,node,L"PloughtToGrassTime");number(beginGrowFrame,node,L"BeginGrowFrame");
    const auto appearance=[&](const char* name,AnimableAppearance& value) {XmlNode* section=nullptr;if(node.section(String(name),section)) value.loadXML(*section,fieldContext_.area.resources);};
    appearance("HideState",hiddenAppearance);appearance("ReadyToBuyState",readyToBuy);appearance("UnderWaterState",underWater);
    XmlNode* section=nullptr;if(node.section(String("AbsorbInfo"),section)) for(unsigned i=0;i<(std::min<std::size_t>)(8,section->children().size());++i) {XmlNode* row=nullptr;if(section->section(i,row)) {pair(absorption[i].water,*row,L"water_absorb_coeffs");pair(absorption[i].fertil,*row,L"fertil_absorb_coeffs");}}
    waterAbsorption=sample(absorption.at(culture()).water);fertilAbsorption=sample(absorption.at(culture()).fertil);
    const auto sound=[&](const wchar_t* name,cResource*& target) {const auto path=toNarrow(node.attribute(WString(name)));if(!path.empty()) {if(!fieldContext_.area.interaction.loadSound) throw std::logic_error("FarmField requires original sound loader");target=fieldContext_.area.interaction.loadSound(path);}};
    sound(L"DeathSound",deathSound_);sound(L"ReadySound",readySound_);
    if(node.section(String("States"),section)) {
        const char* names[]={"GRASS","TOMATOES","MELON","CUCUMBERS","ROSE","WATERMELON","PINEAPPLE","WHEAT"};
        for(unsigned i=0;i<8;++i) {XmlNode* state=nullptr;if(!section->section(String(names[i]),state)) continue;auto& a=cultureAppearances[i];XmlNode* growing=nullptr;if(state->section(String("GROWING"),growing)) a.loadXML(*growing,fieldContext_.area.resources);
            // The original reads these through unspecified stack locals when
            // absent. Actual game XML defines all three; report missing input.
            for(const auto* required:{L"FramePerStage0",L"FramePerStage1",L"NotGrowFrame"}) if(state->attribute(WString(required)).empty()) throw std::logic_error("FarmField culture frame attribute missing; native unspecified input is not recovered");
            auto& info=cultures[i];number(info.firstFrames,*state,L"FramePerStage0");number(info.secondFrames,*state,L"FramePerStage1");number(info.notGrowFrame,*state,L"NotGrowFrame");info.mask=toNarrow(state->attribute(WString(L"texture_mask")));
            const auto frames=originalUnsigned(a.frames);info.notGrowProgress=float(originalDivide(originalMultiply(info.notGrowFrame,fieldContext_.configuration.products[i].creationTime),frames));
            info.firstTime=float(originalDivide(originalMultiply(info.firstFrames,a.length),frames));info.secondTime=float(originalDivide(originalMultiply(info.secondFrames,a.length),frames));
            info.lastTime=float(originalSubtract(originalSubtract(a.length,info.firstTime),info.secondTime));
        }
    }
}
unsigned FarmField::culture() {return fieldContext_.profile.uints[key("FFC")];}
unsigned FarmField::actionState() {return fieldContext_.profile.uints[key("FFS")];}
float FarmField::progress() {return fieldContext_.profile.floats[key("FFP")];}
float FarmField::water() {return fieldContext_.profile.floats[key("FFW")];}
float FarmField::fertility() {return fieldContext_.profile.floats[key("FFF")];}
FieldCultureInfo& FarmField::cultureInfo() {return cultures.at(culture());}
float FarmField::sample(const Point2& range) {return float(originalAdd(originalMultiply(originalMultiply(originalSubtract(range[1],range[0]),fieldContext_.random.next()),3.0518509447574615e-05f),range[0]));}
void FarmField::setProgress(float value) {auto& current=fieldContext_.profile.floats[key("FFP")];if(current!=value) current=value;}
void FarmField::setActionState(unsigned value) {auto& current=fieldContext_.profile.uints[key("FFS")];if(current!=value) current=value;}
void FarmField::setWater(float value) {
    if(value<0) value=0;auto& p=fieldContext_.profile;auto& current=p.floats[key("FFW")];if(current!=value) current=value;
    // Equality remains visible. Unordered values follow the native hide path.
    if(value<=waterSignLevel) waterSign_->show();else waterSign_->hide();
    if(!fieldContext_.world.hasDryField()) {if(!p.booleans["awNoDry"]) {p.booleans["awNoDry"]=true;p.floats["awNoDryTime"]=0;}}
    else {p.booleans["awNoDry"]=false;p.floats["awNoDryTime"]=0;}
    waterAbsorption=sample(absorption.at(culture()).water);
}
void FarmField::setFertility(float value) {
    if(value<0) value=0;auto& current=fieldContext_.profile.floats[key("FFF")];if(current!=value) current=value;
    if(value<=fertilSignLevel) fertilSign_->show();else fertilSign_->hide();fertilAbsorption=sample(absorption.at(culture()).fertil);
}
double FarmField::addWater(float amount) {
    const float free=float(originalSubtract(waterCapacity,water()));if(free<amount) {setWater(waterCapacity);return originalSubtract(amount,free);}
    setWater(float(originalAdd(water(),amount)));return 0;
}
void FarmField::setCulture(unsigned value) {
    if(culture()==value) return;fieldContext_.profile.uints[key("FFC")]=value;refreshAppearance();
    waterAbsorption=sample(absorption.at(value).water);fertilAbsorption=sample(absorption.at(value).fertil);
}
void FarmField::refreshAppearance() {
    auto kind=signedWord(culture());if(kind==-1) kind=0;auto& appearance=cultureAppearances.at(unsigned(kind));
    if(texture()!=appearance.texture) {copyAppearance(appearance);const auto& name=cultures.at(unsigned(kind)).mask;setMaskEnabled(!name.empty());if(!name.empty()) setMask(loadClickMask(name));markTransformDirty();}
}
void FarmField::refreshPurchase(int current,int available) {
    const bool buying=upgradeIndex<=available && signedWord(unsigned(current)+1)==upgradeIndex;
    if(buying || current<upgradeIndex) {hidden_->copyAppearance(buying ? readyToBuy:hiddenAppearance);hidden_->markTransformDirty();hidden_->show();showReadySign(false);return;}
    if(active) return;hidden_->hide();active=true;refreshAppearance();const auto kind=culture();auto remaining=progress();const auto state=actionState();showReadySign(false);
    if(state==0) {
        const auto& product=fieldContext_.configuration.products.at(kind);
        const bool supplied=kind==0 ? water()!=0:fertility()!=0;
        if(!supplied) {setFrame(cultureInfo().notGrowFrame);remaining=float(originalSubtract(product.creationTime,cultureInfo().notGrowProgress));setProgress(remaining);}
        else setTime(float(originalMultiply(originalDivide(originalSubtract(product.creationTime,remaining),product.creationTime),animation_.length)));
        installGrowing(remaining);
    }else if(state==1) {setFrame(animation_.frames-1);showReadySign(true);installReady();}
    else if(state==3) {setFrame(0);installDead(remaining);}
    else {setFrame(0);installPloughed();}
    setWater(water());
}
double FarmField::randomDeathTime() {
    const float lower=float(originalMultiply(ploughedToGrass,-0.05f)),upper=float(originalMultiply(ploughedToGrass,0.05f));
    return originalAdd(originalAdd(originalMultiply(originalMultiply(originalSubtract(upper,lower),fieldContext_.random.next()),3.0518509447574615e-05f),lower),ploughedToGrass);
}
void FarmField::preview(bool value) {if(!active) return;if(value) {hidden_->copyAppearance(underWater);hidden_->markTransformDirty();hidden_->show();}else hidden_->hide();}
void FarmField::showReadySign(bool value) {if(value) readySign_->show();else readySign_->hide();}
void FarmField::playDeath() {if(deathSound_) fieldContext_.area.interaction.playClick(deathSound_);}
void FarmField::playReady() {if(readySound_) fieldContext_.area.interaction.playClick(readySound_);}
void FarmField::advanceAnimation(double amount) {markTransformDirty();animation_.time=float(originalAdd(animation_.time,amount));}
void FarmField::rewindAnimation(double amount) {markTransformDirty();animation_.time=float(originalSubtract(animation_.time,amount));}
void FarmField::enabledStep() {
    auto& c=fieldContext_;if(upgradeIndex>c.profile.ints["fcu"]) return;
    if(fertility()==0 && !c.world.farmScene().movement && shown(c.world.farmScene()) && c.world.tutorial().canShow(49)) c.world.tutorial().showArrow(49,this);
    ExtendedFieldButton::enabledStep();
}
void FarmField::absoluteStep() {
    auto& c=fieldContext_;if(upgradeIndex>c.profile.ints["fcu"]) return;
    const float dt=elapsed();actions_.step();float w=water(),f=fertility();
    if(!c.world.tutorial().activeStep(3) && !c.world.tutorial().activeStep(4)) {
        w=float(originalSubtract(w,originalMultiply(dt,waterAbsorption)));if(w<0) w=0;setWater(w);
        if(actionState()!=1) {f=float(originalSubtract(f,originalMultiply(dt,fertilAbsorption)));if(f<0) f=0;setFertility(f);}
    }
}
void FarmField::onEnabled() {ButtonPrototype::onEnabled();if(pointInRect(fieldContext_.area.interaction.cursor())) hovered();else unhovered();}
void FarmField::onDisabled() {ButtonPrototype::onDisabled();ButtonPrototype::unhovered();if(auto* tip=FieldTip::instance()) tip->hideFor(this);}
void FarmField::clicked() {
    auto& c=fieldContext_;if(c.profile.ints["fcu"]<upgradeIndex) {ButtonPrototype::clicked();return;}
    c.world.setFarmClickHeld(true);const float time=animation_.time;tipsDirection=1;
    if(!c.world.areaGate(64) && !c.world.areaGate(32)) ActionArea::clicked();else ButtonPrototype::clicked();
    setTime(time);if(buttonState()==ButtonState::Pressed) setState(ButtonState::Hovered);
}
void FarmField::unclicked() {if(fieldContext_.profile.ints["fcu"]<upgradeIndex) {ButtonPrototype::unclicked();return;}const float time=animation_.time;ButtonPrototype::unclicked();tipsDirection=1;setTime(time);}
void FarmField::hovered() {
    auto& c=fieldContext_;if(c.profile.ints["fcu"]<upgradeIndex) {ButtonPrototype::hovered();return;}
    if(c.world.farmClickHeld()) enqueueClick();FieldTip::instance()->select(this);c.world.indicateField(*this);const float time=animation_.time;
    // Actual original constructs both query coordinates from animation time.
    if(auto* area=c.world.firstAreaAt({time,time},64)) area->tipsDirection=1;
    ExtendedFieldButton::hovered();tipsDirection=1;setTime(time);
}
void FarmField::unhovered() {
    auto& c=fieldContext_;if(c.profile.ints["fcu"]<upgradeIndex) {ButtonPrototype::unhovered();return;}
    FieldTip::instance()->hideFor(this);const float time=animation_.time;tipsDisabled=false;c.world.clearFieldIndication(*this);ExtendedFieldButton::unhovered();setTime(time);
}
}
