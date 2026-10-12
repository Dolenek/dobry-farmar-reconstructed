#include "TutorialTips.h"
#include "ResourceManager.h"
#include <stdexcept>
#include <cstdint>
#include <limits>
namespace farmar::recovered {
namespace {
float truncateCoordinate(double value) {
    // Native _ftol2 produces a signed 64-bit result, whose low word is reloaded.
    if(!std::isfinite(value) || value>=9223372036854775808.0 || value< -9223372036854775808.0) return 0;
    return float(std::int32_t(std::uint32_t(std::int64_t(value))));
}
}
TutorialCommonTip::TutorialCommonTip(View* parent,TutorialTipContext& context):View(parent,context.views),context_(context) {createBaseControls();}
void TutorialCommonTip::createBaseControls() {
    auto& c=context_;image_=new Picture(this,c.resources,c.views,c.controls);
    text_=new MultilineText(image_,c.resources,c.settings,c.strings,c.views,c.controls);
    ok_=new TextAnimationButton(image_,c.resources,c.settings,c.strings,c.input,c.interaction,c.views,c.controls);ok_->clickEvent=&okClick;
}
void TutorialCommonTip::loadXML(const XmlNode& node) {
    View::loadXML(node);for(auto pair:std::initializer_list<std::pair<const char*,View*>>{{"Image",image_},{"Text",text_},{"OkButton",ok_}}) {XmlNode* section=nullptr;if(node.section(String(pair.first),section)) pair.second->loadXML(*section);}
    defaultMultiplier=text_->multiplier();defaultInterlinear=text_->interlinear();originalPosition={image_->geometry().x,image_->geometry().y};originalDepth=image_->deep();
}
void TutorialCommonTip::updatePlacement() {image_->setPosition({float(originalAdd(originalPosition[0],offset[0])),float(originalAdd(originalPosition[1],offset[1]))});image_->setDeep(originalDepth);}
void TutorialCommonTip::start(const Point2& value,const WString& text) {text_->setString(text,true);offset=value;updatePlacement();hide();showTransparently();}
void TutorialCommonTip::setTextMultiplier(float value) {text_->setMultiplier(value==-1 ? defaultMultiplier:value);}
void TutorialCommonTip::setTextInterlinear(float value) {text_->setInterlinear(value==-1 ? defaultInterlinear:value);}
void TutorialCommonTip::okClick(ButtonPrototype* button) {auto& self=*static_cast<TutorialCommonTip*>(button->parent()->parent());self.hideTransparently();self.context_.host.finishTip(self);}
TutorialArrowTip::TutorialArrowTip(View* parent,TutorialTipContext& context):TutorialCommonTip(parent,context) {
    // Native overwrites its pointers with a second set; the first image subtree
    // remains owned by View. Preserve all seven original allocated controls.
    auto& c=context_;image_=new Picture(this,c.resources,c.views,c.controls);arrow_=new Picture(image_,c.resources,c.views,c.controls);
    text_=new MultilineText(image_,c.resources,c.settings,c.strings,c.views,c.controls);ok_=new TextAnimationButton(image_,c.resources,c.settings,c.strings,c.input,c.interaction,c.views,c.controls);ok_->clickEvent=&okClick;
}
void TutorialArrowTip::loadXML(const XmlNode& node) {TutorialCommonTip::loadXML(node);XmlNode* section=nullptr;if(node.section(String("Arrow"),section)) arrow_->loadXML(*section);}
void TutorialArrowTip::prepare(const Point2& value,const Point2& arrowValue,const WString& text,Control* control) {anchor=control;text_->setString(text,true);offset=value;arrowOffset=arrowValue;updatePlacement();hide();showTransparently();}
void TutorialArrowTip::start(const Point2& value,const Point2& arrowValue,const WString& text,Control* control) {prepare(value,arrowValue,text,control);}
void TutorialArrowTip::updatePlacement() {
    Point2 position=offset,delta=arrowOffset;
    if(anchor) {
        const bool right=anchor->geometry().x>400,bottom=anchor->geometry().y>300;arrow_->setInvertX(right);arrow_->setInvertY(!bottom);
        if(right) {position[0]=-position[0];delta[0]=-delta[0];}if(bottom) {position[1]=-position[1];delta[1]=-delta[1];}
        image_->setPosition({float(originalAdd(originalAdd(position[0],anchor->geometry().x),originalPosition[0])),float(originalAdd(originalAdd(position[1],anchor->geometry().y),originalPosition[1]))});
        arrowOrigin={float(originalAdd(originalAdd(delta[0],anchor->geometry().x),originalPosition[0])),float(originalAdd(float(originalAdd(delta[1],anchor->geometry().y)),originalPosition[1]))};
    } else {arrow_->setInvertX(false);arrow_->setInvertY(true);TutorialCommonTip::updatePlacement();arrowOrigin={float(originalAdd(delta[0],originalPosition[0])),float(originalAdd(delta[1],originalPosition[1]))};}
    const float inverse=float(originalDivide(1,originalSqrt(originalAdd(originalMultiply(delta[1],delta[1]),originalMultiply(delta[0],delta[0])))));
    velocity={float(originalMultiply(delta[0],inverse)),float(originalMultiply(delta[1],inverse))};for(auto& item:velocity) item=float(originalMultiply(item,8));
    const double x=originalAdd(originalMultiply(phase,velocity[0]),arrowOrigin[0]);const float dy=float(originalMultiply(phase,velocity[1]));const double y=originalAdd(dy,arrowOrigin[1]);
    arrow_->setPosition(anchor ? Point2{truncateCoordinate(x),truncateCoordinate(float(y))}:Point2{float(x),float(y)});image_->setDeep(originalDepth);
}
void TutorialArrowTip::enabledStep() {if(!uiElapsedTime()) throw std::logic_error("UI timer is not bound");const auto next=originalAdd(originalMultiply(uiElapsedTime()(),speed),phase);phase=float(next);if(next>1) phase=0;updatePlacement();}
TutorialDisabledAnchorTip::TutorialDisabledAnchorTip(View* parent,TutorialTipContext& context):TutorialArrowTip(parent,context) {ok_->clickEvent=&okClick;}
void TutorialDisabledAnchorTip::start(const Point2& value,const Point2& arrowValue,const WString& text,Control* control) {prepare(value,arrowValue,text,control);anchorWasDisabled=!(control->state().enabled && control->state().parentEnabled);control->disable();}
void TutorialDisabledAnchorTip::okClick(ButtonPrototype* button) {auto& self=*static_cast<TutorialDisabledAnchorTip*>(button->parent()->parent());if(!self.anchorWasDisabled) self.anchor->enable();self.hideTransparently();self.context_.host.finishTip(self);}
TutorialActionTip::TutorialActionTip(View* parent,TutorialTipContext& context):TutorialArrowTip(parent,context) {}
void TutorialActionTip::start(const Point2& value,const Point2& arrowValue,const WString& text,Control* control,Predicate test) {prepare(value,arrowValue,text,control);context_.host.setGameplayPaused(true);predicate=test;if(!test) throw "cActionTips::startup";}
void TutorialActionTip::enabledStep() {TutorialArrowTip::enabledStep();if(predicate(anchor) || context_.host.takeCancelRequest()) {hideTransparently();context_.host.setGameplayPaused(false);context_.host.finishTip(*this);}}
TutorialRepeatedActionTip::TutorialRepeatedActionTip(View* parent,TutorialTipContext& context):TutorialActionTip(parent,context) {}
void TutorialRepeatedActionTip::start(const Point2& value,const Point2& arrowValue,const WString& text,Control* control,Predicate test) {predicate=test;prepare(value,arrowValue,text,control);context_.host.setGameplayPaused(true);if(!test) throw "cActionTips::startup";}
void TutorialRepeatedActionTip::enabledStep() {TutorialActionTip::enabledStep();if(predicate(anchor)) {hideTransparently();context_.host.setGameplayPaused(false);context_.host.finishTip(*this);}}
}
