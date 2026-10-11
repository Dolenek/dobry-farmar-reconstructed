#include "AwardPanel.h"
#include "ResourceManager.h"
#include "XmlDocument.h"
#include "StringConversion.h"
#include "SoundManager.h"
#include "Sound.h"
#include <sstream>
namespace farmar::recovered {
AwardPanel::AwardPanel(AwardEffect& effect,ButtonPrototype::Event accepted,ResourceManager& resources,Settings& settings,
    StringManager& strings,SoundManager& sound,InputBindings& input,ButtonInteraction& interaction,ViewRegistry& views,ControlRegistry& controls)
    :Picture(nullptr,resources,views,controls),effect_(effect),soundManager_(sound) {
    ok_=new TextAnimationButton(this,resources,settings,strings,input,interaction,views,controls);ok_->clickEvent=accepted;
    text_=new MultilineText(this,resources,settings,strings,views,controls);
    congratulation_=new MultilineText(this,resources,settings,strings,views,controls);
    const auto xml=resources.loadSettingsXML(String("UI/AwardPanel.xml"));loadXML(*xml);
}
AwardPanel::~AwardPanel() { if(sound_) sound_->releaseReference(); }
void AwardPanel::loadXML(const XmlNode& node) {
    const auto position=node.attribute(WString(L"AwardPicPosition"));
    if(!position.empty()) {std::wistringstream stream(position.c_str());stream>>awardPicturePosition_[0];stream.ignore(1);stream>>awardPicturePosition_[1];}
    XmlNode* child=nullptr;
    if(node.section(String("OkButton"),child)) ok_->loadXML(*child);
    if(node.section(String("Text"),child)) text_->loadXML(*child);
    if(node.section(String("CongratulationText"),child)) congratulation_->loadXML(*child);
    text_->setString(effect_.effectState().panelText);congratulation_->setString(effect_.effectState().congratulation);
    const auto name=node.attribute(WString(L"Sound"));
    // Original overwrites this slot without releasing any earlier Sound.
    if(!name.empty()) sound_=resources_.getSound(toNarrow(name));
    Picture::loadXML(node); // Original loads root appearance after owned control XML and both label assignments.
}
void AwardPanel::onShown() { Picture::onShown();if(sound_) soundManager_.clickSound(sound_); }
}
