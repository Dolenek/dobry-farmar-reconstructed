#include "YesNoDialog.h"
#include "ResourceManager.h"
namespace farmar::recovered {
YesNoDialog::YesNoDialog(View* suspended,ResourceManager& resources,Settings& settings,StringManager& strings,
    InputBindings& input,ButtonInteraction& interaction,ViewRegistry& views,ControlRegistry& controls)
    :Picture(nullptr,resources,views,controls),input_(input),suspended_(suspended),
    yes_(new TextAnimationButton(this,resources,settings,strings,input,interaction,views,controls)),
    no_(new TextAnimationButton(this,resources,settings,strings,input,interaction,views,controls)),
    text_(new MultilineText(this,resources,settings,strings,views,controls)) {
    yes_->clickEvent=&yesClicked;no_->clickEvent=&noClicked;
    const auto document=resources.loadSettingsXML(String("UI/YesNo.xml"));loadXML(*document);
}
YesNoDialog::~YesNoDialog() { if(state_.enabled && state_.parentEnabled) onDisabled(); }
void YesNoDialog::loadXML(const XmlNode& node) {
    XmlNode* child=nullptr;
    if(node.section(String("Background"),child)) Picture::loadXML(*child);
    if(node.section(String("YesButton"),child)) yes_->loadXML(*child);
    if(node.section(String("NoButton"),child)) no_->loadXML(*child);
    if(node.section(String("Text"),child)) text_->loadXML(*child);
}
void YesNoDialog::onEnabled() {
    View::onEnabled();if(suspended_) suspended_->disable();
    input_.bind(0x1c,&yesInput,yes_);input_.bind(0x39,&yesInput,yes_);input_.bind(1,&noInput,no_);
}
void YesNoDialog::onDisabled() {
    View::onDisabled();if(suspended_) suspended_->enable();
    input_.unbind(0x1c,&yesInput,yes_);input_.unbind(0x39,&yesInput,yes_);input_.unbind(1,&noInput,no_);
}
void YesNoDialog::enabledStep() {
    Picture::enabledStep();if(suspended_ && suspended_->state().enabled && suspended_->state().parentEnabled) suspended_->disable();
}
void YesNoDialog::yesClicked(ButtonPrototype* button) { auto* self=static_cast<YesNoDialog*>(button->parent());self->yesEvent_(self); }
void YesNoDialog::noClicked(ButtonPrototype* button) { auto* self=static_cast<YesNoDialog*>(button->parent());self->noEvent_(self); }
void YesNoDialog::yesInput(void* button) { yesClicked(static_cast<ButtonPrototype*>(button)); }
void YesNoDialog::noInput(void* button) { noClicked(static_cast<ButtonPrototype*>(button)); }
void YesNoDialog::discard(YesNoDialog* self) { self->registry_.requestDiscard(self,String(".\\Ui\\YesNo.cpp45")); }
}
