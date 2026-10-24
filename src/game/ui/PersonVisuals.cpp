#include "PersonVisuals.h"
#include "Resource.h"
#include "XmlDocument.h"
#include "StringConversion.h"
#include "OriginalArithmetic.h"
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <cmath>
namespace farmar::recovered {
namespace {
template<class T> void attribute(const XmlNode& node,const wchar_t* key,T& value) {
    const auto text=node.attribute(WString(key));if(!text.empty()) {std::wistringstream stream(text.c_str());stream>>value;}
}
void point(const XmlNode& node,const wchar_t* key,Point2& value) {
    const auto text=node.attribute(WString(key));if(!text.empty()) {std::wistringstream stream(text.c_str());stream>>value[0];stream.ignore(1);stream>>value[1];}
}
template<class Function> void rows(const XmlNode& node,const char* key,unsigned maximum,Function function) {
    XmlNode* section=nullptr;if(!node.section(String(key),section)) return;
    for(unsigned i=0;i<(std::min)(maximum,unsigned(section->children().size()));++i) {XmlNode* row=nullptr;if(section->section(i,row)) function(i,*row);}
}
void child(View& view,const XmlNode& node,const char* key) {XmlNode* part=nullptr;if(node.section(String(key),part)) view.loadXML(*part);}
double personAcos(float value) {
    // 004497e0 / 0044fe20 catch unordered values in the first x87 comparison.
    constexpr float pi=3.1415927f;if(!(value>-1)) return pi;if(!(value<1)) return 0;
#if defined(_MSC_VER) && defined(_M_IX86)
    // 00588535 keeps the caller's precision bits, masks exceptions and selects
    // nearest rounding. 0058106d computes atan2(sqrt((1-x)*(1+x)),x) with x87.
    unsigned short savedControl;double result;
    __asm fnstcw savedControl
    const unsigned short mathControl=(savedControl&0x0300)|0x007f;
    __asm {
        fldcw mathControl
        fld value
        fld1
        fadd st,st(1)
        fld1
        fsub st,st(2)
        fmulp st(1),st
        fsqrt
        fxch st(1)
        fpatan
        fstp result
        fldcw savedControl
    }
    return result;
#else
    return std::acos(double(value));
#endif
}
}
PersonVisuals::PersonVisuals(FieldObject& owner,PersonVisualContext& c):owner_(owner),context_(c) {
    waterUse_=new ProgressBar(&owner,c.resources,c.views,c.controls);
    discreteUse_=new ProgressDiscrete(&owner,c.resources,c.views,c.controls);
    sign_=new Picture(&owner,c.resources,c.views,c.controls);substrate_=new Picture(sign_,c.resources,c.views,c.controls);
}
PersonVisuals::~PersonVisuals() {releaseSounds();}
void PersonVisuals::releaseSounds() {for(auto*& sound:sounds) if(sound) {sound->releaseReference();sound=nullptr;}}
void PersonVisuals::loadXML(const XmlNode& node) {
    owner_.FieldObject::loadXML(node);attribute(node,L"Speed",speed);point(node,L"MarkSize",markerSize);
    child(*waterUse_,node,"Use");child(*discreteUse_,node,"DiscrUse");child(*sign_,node,"WotInScythSign");child(*substrate_,node,"WotSubstrate");
    XmlNode* section=nullptr;if(node.section(String("moveMarkState"),section)) markerAppearance.loadXML(*section,context_.resources);
    if(node.section(String("AnimationConfig"),section)) {
        const wchar_t* keys[]={L"sowTime",L"waterPotTime",L"dropWareTime",L"dropSpadeTime",L"dropScytheTime",L"dropHoseTime",L"getSpadeTime",L"getScytheTime",L"getBucketTime",L"getHoseTime"};
        for(unsigned i=0;i<10;++i) attribute(*section,keys[i],actionTimes[i]);
    }
    const auto appearances=[&](const char* key,auto& values,unsigned count) {
        rows(node,key,count,[&](unsigned i,const XmlNode& row) {for(unsigned j=0;j<(std::min)(6U,unsigned(row.children().size()));++j) {XmlNode* part=nullptr;if(row.section(j,part)) values[i*6+j].loadXML(*part,context_.resources);}});
    };
    appearances("States",moving,12);appearances("JobStates",working,11);appearances("IdleStates",idle,12);
    if(node.section(String("StateWaterWithHose"),section)) hose.loadXML(*section,context_.resources);
    hotSpots.fill(Point2{});
    rows(node,"HotSpots",12,[&](unsigned i,const XmlNode& row) {for(unsigned j=0;j<(std::min)(6U,unsigned(row.children().size()));++j) {XmlNode* part=nullptr;if(row.section(j,part)) {Point2 offset{};point(*part,L"spotOffset",offset);auto& target=hotSpots[i*6+j];target={float(originalAdd(offset[0],target[0])),float(originalAdd(offset[1],target[1]))};}}});
    owner_.setPosition({owner_.geometry().x,owner_.geometry().y});
    if(node.section(String("Sounds"),section)) {
        const wchar_t* keys[]={L"WorkShovelSound",L"WorkTractorSound",L"WorkWateringCanSound",L"WorkSeedSound",L"WorkScytheSound",L"WorkWellSound",L"WorkHoseSound",L"StuffWarehouseSound"};
        for(unsigned i=0;i<8;++i) {const auto path=toNarrow(section->attribute(WString(keys[i])));if(!path.empty()) {
            if(!context_.interaction.loadSound) throw std::logic_error("Person requires actual sound resource loader");
            sounds[i]=context_.interaction.loadSound(path); // original reload retains old references
        }}
    }
    rows(node,"CarryHeightOffsets",12,[&](unsigned i,const XmlNode& row) {attribute(row,L"height",carryHeight[i]);});
    rows(node,"JobHeightOffsets",11,[&](unsigned i,const XmlNode& row) {for(unsigned j=0;j<(std::min)(6U,unsigned(row.children().size()));++j) {XmlNode* part=nullptr;if(row.section(j,part)) attribute(*part,L"height",jobHeight[i*6+j]);}});
    rows(node,"WaterEmitterOfset",64,[&](unsigned i,const XmlNode& row) {point(row,L"offset",waterEmitterOffsets[i]);});
    rows(node,"WaterAngleOfset",64,[&](unsigned i,const XmlNode& row) {attribute(row,L"angle",waterAngles[i]);});
    rows(node,"ProductTipsOffset",6,[&](unsigned i,const XmlNode& row) {point(row,L"offset",productTips[i]);});
    rows(node,"SeedTipsOffset",6,[&](unsigned i,const XmlNode& row) {point(row,L"offset",seedTips[i]);});
}
unsigned PersonVisuals::carryIndex() {
    auto carry=context_.profile.uints["PCarry"];
    if(carry==2) carry=unsigned(context_.world.digLevel())+2;else if(carry==6) carry=unsigned(context_.world.mowLevel())+6;
    return carry;
}
void PersonVisuals::copyIfTextureChanged(const AnimableAppearance& a) {if(owner_.texture()!=a.texture) {owner_.copyAppearance(a);owner_.markTransformDirty();}}
void PersonVisuals::applyHeight(float value) {owner_.heightOffset=value;owner_.setPosition({owner_.geometry().x,owner_.geometry().y});}
void PersonVisuals::refreshHeldAppearance() {
    if(activity!=12) {playJob(activity);return;}const auto carry=carryIndex();
    copyIfTextureChanged((hoseAnimation ? idle:moving).at(carry*6+direction));applyHeight(carryHeight.at(carry));
}
void PersonVisuals::playIdle() {
    hoseAnimation=true;const auto carry=carryIndex();
    if(carry==11) {owner_.copyAppearance(hose);owner_.setFrame(hoseFrame);owner_.markTransformDirty();}
    else copyIfTextureChanged(idle.at(carry*6+direction));
    const auto raw=context_.profile.uints["PCarry"];
    if(raw==6) {const auto& offset=productTips.at(direction);sign_->setPosition({float(owner_.geometry().invertX ? originalSubtract(owner_.geometry().x,offset[0]):originalAdd(owner_.geometry().x,offset[0])),float(originalAdd(owner_.geometry().y,offset[1]))});}
    else if(raw==10) {const auto& offset=seedTips.at(direction);sign_->setPosition({float(originalAdd(owner_.geometry().x,offset[0])),float(originalAdd(owner_.geometry().y,offset[1]))});}
    applyHeight(carryHeight.at(carry));activity=12;
}
void PersonVisuals::playJob(unsigned job) {
    hoseAnimation=false;if(job==0) job=unsigned(context_.world.digLevel());if(job==4) job=unsigned(context_.world.mowLevel())+4;
    if(job!=1 && job!=2 && job!=3 && job!=4 && job!=5 && job!=6 && job!=7 && job!=10) owner_.setInvertX(false);
    const auto index=job*6+direction;copyIfTextureChanged(working.at(index));applyHeight(jobHeight.at(index));activity=job;
}
void PersonVisuals::playWalk(const Point2& delta) {
    const auto squared=originalAdd(originalMultiply(delta[1],delta[1]),originalMultiply(delta[0],delta[0]));
    const float scale=float(originalDivide(1,originalSqrt(squared)));
    const float x=float(originalMultiply(delta[0],scale));const auto y=originalMultiply(delta[1],scale);
    const float dot=float(originalSubtract(originalMultiply(x,0),originalMultiply(y,1)));
    hoseAnimation=false;owner_.setInvertX(delta[0]>0);const auto carry=carryIndex();const auto angle=personAcos(dot);
    constexpr float pi=3.1415927f;unsigned facing=0;
    for(double part:{.125,.375,.625,.875}) {if(angle<originalMultiply(pi,part)) break;++facing;}
    copyIfTextureChanged(moving.at(carry*6+facing));direction=facing;
    const auto& offset=seedTips.at(facing);sign_->setPosition({float(originalAdd(owner_.geometry().x,offset[0])),float(originalAdd(owner_.geometry().y,offset[1]))});
    activity=12;applyHeight(carryHeight.at(carry));
}
void PersonVisuals::playAxisWalk(const Point2& delta) {
    hoseAnimation=false;const auto carry=carryIndex();
    if(delta[1]==0) {if(delta[0]!=0) {owner_.setInvertX(delta[0]>0);direction=2;}}
    else direction=!(delta[1]<0) ? 4:0;
    copyIfTextureChanged(moving.at(carry*6+direction));const auto& offset=productTips.at(direction);
    sign_->setPosition({float(owner_.geometry().invertX ? originalSubtract(owner_.geometry().x,offset[0]):originalAdd(owner_.geometry().x,offset[0])),float(originalAdd(owner_.geometry().y,offset[1]))});
    activity=12;applyHeight(carryHeight.at(carry));
}
void PersonVisuals::playHose(const Point2& target) {
    hoseAnimation=true;owner_.setInvertX(false);
    const float dy=float(originalSubtract(target[1],owner_.geometry().y));const auto dx=originalAdd(originalSubtract(target[0],owner_.geometry().x),15);
    const auto scale=originalDivide(1,originalSqrt(originalAdd(originalMultiply(dx,dx),originalMultiply(dy,dy))));
    const auto x=originalMultiply(scale,dx);const float y=float(originalMultiply(float(scale),dy));
    const float dot=float(originalSubtract(originalMultiply(x,0),originalMultiply(y,1)));hoseFrame=hose.frames-1;
    if(x<=0 && y>=0) {constexpr float pi=3.1415927f;const auto halfPi=originalMultiply(pi,.5);
        const auto shift=originalDivide(originalMultiply(originalSubtract(personAcos(dot),halfPi),originalUnsigned(hose.frames-1)),halfPi);
        hoseFrame-=unsigned(static_cast<long long>(shift));
    }
    owner_.copyAppearance(hose);owner_.setFrame(hoseFrame);owner_.markTransformDirty();activity=12;applyHeight(carryHeight.at(11));
}
unsigned PersonVisuals::carriedProducts() {
    unsigned total=0;for(unsigned i=0;i<8;++i) total+=context_.profile.uints["PCC"+std::to_string(i)];return total;
}
void PersonVisuals::showDiscrete(unsigned char enabled,unsigned current,unsigned maximum) {
    if(enabled!=1) {sign_->hide();discreteUse_->hide();return;}
    const float width=discreteUse_->pictureOn().geometry().width;
    float length=float(originalMultiply(originalUnsigned(maximum),width));Point2 position{discreteUse_->geometry().x,discreteUse_->geometry().y};
    if(!(length>50)) position[0]=float(originalSubtract(owner_.geometry().x,originalMultiply(originalMultiply(originalUnsigned(maximum-1),width),.5)));
    else {length=50;position[0]=float(originalAdd(originalMultiply(width,.5),originalSubtract(owner_.geometry().x,25)));}
    discreteUse_->setPosition(position);discreteUse_->setMaximum(maximum);
    if(discreteUse_->current!=current) {discreteUse_->current=current;discreteUse_->refresh();}
    if(discreteUse_->maxLength!=length) {discreteUse_->maxLength=length;discreteUse_->refresh();}discreteUse_->show();
    auto& c=context_;
    if(c.profile.uints["PCarry"]==6 && c.world.mowLevel()<3 && carriedProducts()!=0) {
        for(unsigned i=0;i<8;++i) if(c.profile.uints["PCC"+std::to_string(i)]!=0) {sign_->copyAppearance(c.configuration.products[i].icon);break;}sign_->show();
    }else if(c.profile.uints["PCarry"]==10 && c.world.carriedSeedCount()!=0) {sign_->copyAppearance(c.configuration.products.at(c.world.carriedSeedKind()).icon);sign_->show();}
    // The original final predicate also hides a newly selected seed icon when
    // the mower/product condition is false. Preserve that observable ordering.
    if(c.world.mowLevel()<3 && carriedProducts()!=0) return;sign_->hide();
}
Point2 PersonVisuals::rawHotSpot(unsigned facing) {return hotSpots.at(carryIndex()*6+facing);}
Point2 PersonVisuals::hotSpotOffset(unsigned facing) {auto result=rawHotSpot(facing);if(owner_.geometry().invertX) result[0]=-result[0];return result;}
Point2 PersonVisuals::hotSpot() {const auto offset=hotSpotOffset(direction);return {float(originalAdd(owner_.geometry().x,offset[0])),float(originalAdd(owner_.geometry().y,offset[1]))};}
}
