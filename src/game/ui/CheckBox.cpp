#include "CheckBox.h"
#include "XmlDocument.h"
#include <sstream>
namespace farmar::recovered {
CheckBox::CheckBox(View* parent,ResourceManager& resources,InputBindings& input,ButtonInteraction& interaction,ViewRegistry& views,ControlRegistry& controls)
    :Button(parent,resources,input,interaction,views,controls),tick_(new Picture(this,resources,views,controls)) {}
void CheckBox::updateTick() {
    const bool visible=tick_->state().shown && tick_->state().parentShown;
    if(checked_!=visible) { if(checked_) tick_->show();else tick_->hide(); }
}
void CheckBox::loadXML(const XmlNode& node) {
    Button::loadXML(node);XmlNode* entry=nullptr;if(node.section(String("Tick"),entry)) tick_->loadXML(*entry);
    const auto value=node.attribute(WString(L"Checked"));if(!value.empty()) { std::wistringstream stream(value.c_str());stream>>checked_; }
    updateTick();
}
void CheckBox::enabledStep() { ButtonPrototype::enabledStep();updateTick(); }
void CheckBox::unclicked() { ButtonPrototype::unclicked();checked_=!checked_;if(changed) changed(this,checked_); }
}
