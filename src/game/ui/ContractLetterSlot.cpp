#include "ContractLetterSlot.h"
#include "TutorialManager.h"
#include "StateView.h"
#include "ProfileProperties.h"
#include "ResourceManager.h"
#include "XmlDocument.h"
#include "SoundManager.h"
#include "Sound.h"
#include "StringConversion.h"
#include "OriginalArithmetic.h"
#include <sstream>
#include <stdexcept>
namespace farmar::recovered {
namespace {
template<class T> void attribute(const XmlNode& node,const wchar_t* key,T& value) {
    const auto text=node.attribute(WString(key));if(!text.empty()) {std::wistringstream stream(text.c_str());stream>>value;}
}
Point2 scaleTo(Point2 value,float length) { //00421520; X uses unspilled multiplier, Y reloads float
    if(value[0]!=0 || value[1]!=0) {
        const auto factor=originalDivide(length,originalSqrt(originalAdd(originalMultiply(value[1],value[1]),originalMultiply(value[0],value[0]))));
        value={float(originalMultiply(factor,value[0])),float(originalMultiply(float(factor),value[1]))};
    }return value;
}
}
ContractLetterSlot::ContractLetterSlot(View* parent,unsigned index,ContractLetterContext& c)
    :ExtendedAnimationButton(parent,c.resources,c.settings,c.strings,c.input,c.interaction,[&c]{return c.world.tutorial().active();},c.views,c.controls),id(index),context_(c) {
    image_=new AnimationButton(this,c.resources,c.input,c.interaction,c.views,c.controls);
    progress_=new ProgressBar(this,c.resources,c.views,c.controls);
    loadXML(*c.resources.loadSettingsXML(String("UI/gameUI/Farm/ContractLetterSlot.xml")));
}
ContractLetterSlot::~ContractLetterSlot() {if(nearSound_) nearSound_->releaseReference();if(expiredSound_) expiredSound_->releaseReference();}
std::string ContractLetterSlot::key(const char* prefix) const {return std::string(prefix)+std::to_string(id);}
GameplayEvent& ContractLetterSlot::contractEvent() const {if(!event) throw std::logic_error("Contract letter requires its assigned event");return *event;}
void ContractLetterSlot::loadXML(const XmlNode& node) {
    ExtendedAnimationButton::loadXML(node);XmlNode* child=nullptr;
    if(node.section(String("FloatProgress"),child)) progress_->loadXML(*child);
    if(node.section(String("Pic"),child)) image_->loadXML(*child);
    attribute(node,L"FlashTimeRemaind",warningTime);attribute(node,L"FlashPeriod",blinkPeriod);
    const auto at=node.attribute(WString(L"AppearPosition"));if(!at.empty()) {std::wistringstream stream(at.c_str());stream>>appear[0]>>appear[1];}
    attribute(node,L"MoveSpeed",moveSpeed);
    if(node.section(String("ReadState"),child)) read_.loadXML(*child,context_.resources);
    if(node.section(String("UnreadState"),child)) unread_.loadXML(*child,context_.resources);
    // Original XML reload replaces pointers without releasing old references.
    const auto nearPath=toNarrow(node.attribute(WString(L"NearPuffSound"))),expired=toNarrow(node.attribute(WString(L"PuffSound")));
    if(!nearPath.empty()) nearSound_=context_.resources.getSound(nearPath);
    if(!expired.empty()) expiredSound_=context_.resources.getSound(expired);
}
void ContractLetterSlot::updateProgress() {progress_->current=float(originalMultiply(originalDivide(timeRemaining,fullDuration),100));progress_->updateLid();}
void ContractLetterSlot::assign(GameplayEvent& value,unsigned state,bool restored) {
    event=&value;const auto& time=std::get<ContractEvent>(value.data).letterTime;
    fullDuration=float(originalUnsigned((time.hours*60U+time.minutes)*60U+time.seconds));
    if(!restored) {
        const auto dx=originalSubtract(place[0],appear[0]),dy=originalSubtract(place[1],appear[1]);
        moveRemaining=float(originalSqrt(originalAdd(originalMultiply(dx,dx),originalMultiply(dy,dy))));
        disable();const auto& origin=context_.world.farmScene().movingPicture().geometry();
        setPosition({float(originalAdd(origin.x,appear[0])),float(originalAdd(origin.y,appear[1]))});
        letterState=state;timeRemaining=fullDuration;warning=false;
        context_.profile.ints[key("letter")]=static_cast<int>(value.index);
        context_.profile.floats[key("letTime")]=timeRemaining;context_.profile.ints[key("letState")]=static_cast<int>(letterState);
    } else {
        moveRemaining=0;enable();setPosition(place);
        timeRemaining=context_.profile.floats[key("letTime")];letterState=unsigned(context_.profile.ints[key("letState")]);warning=!(warningTime<timeRemaining);
    }
    if(letterState==1) image_->copyAppearance(read_);else if(letterState==0) image_->copyAppearance(unread_);
    updateProgress();image_->show();show();
}
void ContractLetterSlot::absoluteStep() {
    View::absoluteStep();auto& tutorial=context_.world.tutorial();const float dt=tutorial.active() ? 0:uiElapsedTime()();
    if(moveRemaining>0) {
        const auto left=originalSubtract(moveRemaining,originalMultiply(dt,moveSpeed));moveRemaining=float(left);if(left<0) {moveRemaining=0;enable();}
        auto delta=scaleTo({float(originalSubtract(appear[0],place[0])),float(originalSubtract(appear[1],place[1]))},moveRemaining);
        const auto& origin=context_.world.farmScene().movingPicture().geometry();
        // X addition spills before adding the farm origin; Y stays in x87.
        setPosition({float(originalAdd(float(originalAdd(delta[0],place[0])),origin.x)),float(originalAdd(originalAdd(delta[1],place[1]),origin.y))});return;
    }
    if(letterState==2) return;
    const auto left=originalSubtract(timeRemaining,dt);timeRemaining=float(left);if(left<0) timeRemaining=0;
    context_.profile.floats[key("letTime")]=timeRemaining;
    if(timeRemaining<90 && tutorial.canShow(17) && context_.world.farmScene().movement==0 && state().shown && state().parentShown) tutorial.showAction(17,this);
    if(!warning) {if(timeRemaining<=warningTime) {warning=true;blinkRemaining=0;if(nearSound_) context_.sound.clickSound(nearSound_);}}
    else {const auto next=originalSubtract(blinkRemaining,dt);blinkRemaining=float(next);if(next<=0) {blinkRemaining=blinkPeriod;if(!image_->state().shown) image_->show();else image_->hide();}}
    if(timeRemaining==0) {context_.world.removeLetter(*this);if(expiredSound_) context_.sound.clickSound(expiredSound_);context_.profile.uints["awQTask"]=0;}
    else updateProgress();
}
void ContractLetterSlot::clicked() {
    ExtendedAnimationButton::clicked();auto& tutorial=context_.world.tutorial();if(tutorial.active() && !tutorial.activeStep(17)) return;
    tutorial.complete(17);
    if(letterState==0) {letterState=1;image_->copyAppearance(read_);context_.profile.ints[key("letState")]=1;}
    if(warning) {blinkRemaining=blinkPeriod;image_->show();}
    auto* offer=ContractProvide::instance();if(!offer) throw std::logic_error("Contract letter requires actual ContractProvide");offer->start(*this);
}
}
