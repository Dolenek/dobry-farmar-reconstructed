#include "ExitDialog.h"
#include "ResourceManager.h"
#include "XmlDocument.h"
#include "OriginalArithmetic.h"
#include <stdexcept>
namespace farmar::recovered {
namespace {
// 00421520 retains the extended ratio for X but reloads its float spill for Y.
void setVectorLength(Point2& point,float targetLength) {
    const float x=point[0],y=point[1];if(x==0 && y==0) return;
    float outputX,outputY,ratio;
#if defined(_MSC_VER) && defined(_M_IX86)
    __asm {
        fld y
        fld x
        fld st(1)
        fmulp st(2),st(0)
        fld st(0)
        fmul st(0),st(1)
        faddp st(2),st(0)
        fxch st(1)
        fsqrt
        fstp st(1)
        fdivr targetLength
        fst ratio
        fmul x
        fstp outputX
        fld ratio
        fmul y
        fstp outputY
    }
#else
    const auto extendedRatio=originalDivide(targetLength,originalSqrt(originalAdd(originalMultiply(x,x),originalMultiply(y,y))));
    ratio=static_cast<float>(extendedRatio);outputX=static_cast<float>(originalMultiply(extendedRatio,x));outputY=static_cast<float>(originalMultiply(ratio,y));
#endif
    point={outputX,outputY};
}
ExitDialog* owner(ButtonPrototype* button) { return static_cast<ExitDialog*>(button->parent()); }
}
ExitDialog::ExitDialog(View* parent,ResourceManager& resources,Settings& settings,StringManager& strings,
    InputBindings& input,ButtonInteraction& interaction,ViewRegistry& views,ControlRegistry& controls)
    :MenuView(parent,resources,settings,strings,views,controls),resources_(resources),interaction_(interaction) {
    for(auto& bee:bees_) bee=new Picture(this,resources,views,controls);
    yes_=new TextAnimationButton(this,resources,settings,strings,input,interaction,views,controls);
    no_=new TextAnimationButton(this,resources,settings,strings,input,interaction,views,controls);
    text_=new MultilineText(this,resources,settings,strings,views,controls);
    yes_->clickEvent=&yesClick;no_->clickEvent=&noClick;yes_->hoverEvent=&yesHover;no_->hoverEvent=&noHover;yes_->unhoverEvent=no_->unhoverEvent=&leave;
    const auto document=resources.loadSettingsXML(String("UI/YesNoExit.xml"));loadXML(*document);originalYes_={yes_->geometry().x,yes_->geometry().y};bees_[0]->show();bees_[1]->hide();bees_[2]->hide();
}
void ExitDialog::loadXML(const XmlNode& node) {
    MenuView::loadXML(node);const auto load=[&](const char* key,View* control) { XmlNode* child=nullptr;if(node.section(String(key),child)) control->loadXML(*child); };
    load("YesButton",yes_);load("NoButton",no_);load("Text",text_);load("Bee0",bees_[0]);load("Bee1",bees_[1]);load("Bee2",bees_[2]);
}
void ExitDialog::hideDialog(ExitDialog* dialog) { dialog->hide(); }
void ExitDialog::yesClick(ButtonPrototype* button) { auto* self=owner(button);self->yesEvent(self); }
void ExitDialog::noClick(ButtonPrototype* button) { auto* self=owner(button);self->noEvent(self); }
void ExitDialog::yesHover(ButtonPrototype* button) { auto* self=owner(button);self->bees_[0]->hide();self->bees_[1]->hide();self->bees_[2]->show(); }
void ExitDialog::noHover(ButtonPrototype* button) { auto* self=owner(button);self->bees_[0]->hide();self->bees_[1]->show();self->bees_[2]->hide(); }
void ExitDialog::leave(ButtonPrototype* button) { if(button->buttonState()==ButtonState::Pressed) return;auto* self=owner(button);self->bees_[0]->show();self->bees_[1]->hide();self->bees_[2]->hide(); }
void ExitDialog::enabledStep() {
    if(yes_->buttonState()==ButtonState::Hovered) return;
    if(!interaction_.cursor) throw std::logic_error("ExitDialog cursor service is not bound");
    const auto cursor=interaction_.cursor();
    const auto x=originalSubtract(yes_->geometry().x,cursor[0]),y=originalSubtract(yes_->geometry().y,cursor[1]);
    Point2 direction{static_cast<float>(x),static_cast<float>(y)};
    float repulsion=static_cast<float>(originalMultiply(originalSubtract(1,originalMultiply(originalSqrt(originalAdd(originalMultiply(x,x),originalMultiply(y,y))),0.0125f)),30));
    if(repulsion<0) repulsion=0;setVectorLength(direction,repulsion);
    direction={static_cast<float>(originalSubtract(originalAdd(direction[0],originalYes_[0]),yes_->geometry().x)),static_cast<float>(originalSubtract(originalAdd(direction[1],originalYes_[1]),yes_->geometry().y))};
    const float squared=static_cast<float>(originalAdd(originalMultiply(direction[0],direction[0]),originalMultiply(direction[1],direction[1])));
    const float distance=static_cast<float>(originalSqrt(squared));const auto speed=originalMultiply(uiElapsedTime() ? uiElapsedTime()():0,120);
    const float step=static_cast<float>(speed<=distance ? originalMultiply(uiElapsedTime() ? uiElapsedTime()():0,120):originalSqrt(squared));
    setVectorLength(direction,step);yes_->setPosition({static_cast<float>(originalAdd(direction[0],yes_->geometry().x)),static_cast<float>(originalAdd(direction[1],yes_->geometry().y))});
}
}
