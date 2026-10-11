#include "AwardButton.h"
#include "ResourceManager.h"
#include "Texture.h"
#include "XmlDocument.h"
#include "StringConversion.h"
namespace farmar::recovered {
AwardButton::AwardButton(View* parent,unsigned index,const WString& description,ResourceManager& resources,
    Settings& settings,StringManager& strings,SoundManager& sound,InputBindings& input,ButtonInteraction& interaction,
    TooltipFrame::ScreenSize screenSize,ViewRegistry& views,ControlRegistry& controls)
    :ButtonPrototype(parent,resources,input,interaction,views,controls),index_(index),description_(description) {
    tooltip_=new TooltipFrame(this,resources,settings,strings,std::move(screenSize),views,controls);
    effect_=new AwardEffect(this,index,resources,sound,views,controls);effect_->hide();effect_->setPhase(5);
}
AwardButton::~AwardButton() { if(normal_) normal_->releaseReference();if(disabled_) disabled_->releaseReference(); }
void AwardButton::setOwned(unsigned char value) { owned_=value;onStateChanged(value ? ButtonState::Normal:ButtonState::Disabled); }
void AwardButton::onStateChanged(ButtonState state) {
    cTexture* selected=nullptr;
    switch(state) {
    case ButtonState::Disabled:case ButtonState::Hovered:case ButtonState::Pressed:flags_&=~0x20U;selected=disabled_;break;
    case ButtonState::Normal:flags_|=0x20;selected=normal_;break;
    default:dirty_=true;return;
    }
    auto* previous=texture_;if(previous) previous->releaseReference();
    // The native null-state-resource path re-retains the old pointer after its
    // release. It does not clear a previously selected texture.
    texture_=selected ? selected:previous;if(texture_) texture_->addReference();dirty_=true;
}
void AwardButton::onEnabled() { ButtonPrototype::onEnabled();onStateChanged(owned_ ? ButtonState::Normal:ButtonState::Disabled); }
void AwardButton::onDisabled() { ButtonPrototype::onDisabled();tooltip_->fadeDirection=-1;onStateChanged(owned_ ? ButtonState::Normal:ButtonState::Disabled); }
void AwardButton::hovered() {
    ButtonPrototype::hovered();onStateChanged(ButtonState::Disabled);
    if(state_.enabled && state_.parentEnabled) {
        tooltip_->fadeDirection=1;tooltip_->show();
        if(owned_) { effect_->show();effect_->setPhase(6);effect_->setPosition({geometry_.x,geometry_.y}); }
    }
}
void AwardButton::unhovered() {
    ButtonPrototype::unhovered();if(!owned_ || effect_->state().shown) onStateChanged(ButtonState::Disabled);
    tooltip_->fadeDirection=-1;if(owned_ && effect_->state().shown) effect_->setPhase(8);
}
void AwardButton::enabledStep() {
    if(effect_->state().shown && buttonState_!=ButtonState::Hovered && effect_->effectState().phase==5) { effect_->hide();onStateChanged(ButtonState::Normal); }
    ButtonPrototype::enabledStep();
}
void AwardButton::loadXML(const XmlNode& node) {
    ButtonPrototype::loadXML(node);
    // Like native XML, overwriting these two slots retains older references.
    // Destructor releases the latest pair. Repeated reload is compared separately.
    normal_=texture_;if(normal_) normal_->addReference();
    const auto name=toNarrow(node.attribute(WString(L"texture_disabled")));
    if(name.empty()) { disabled_=texture_;if(disabled_) disabled_->addReference(); }else disabled_=resources_.getTexture(name);
    const auto xml=resources_.loadSettingsXML(String("UI/AwardTip.xml"));tooltip_->loadXML(*xml);tooltip_->hide();
    tooltip_->text().setString(description_);tooltip_->placeBeside({geometry_.x,geometry_.y});tooltip_->enable();
}
}
