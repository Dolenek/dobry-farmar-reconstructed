#include "Awards.h"
#include "ResourceManager.h"
#include "XmlDocument.h"
#include "ProfileProperties.h"
#include "AwardPanel.h"
#include <algorithm>
#include <string>
namespace farmar::recovered {
namespace { Awards* currentAwards=nullptr; }
AwardsLifetime::AwardsLifetime(Awards* owner) { if(currentAwards) throw "Object already exist.";currentAwards=owner; }
AwardsLifetime::~AwardsLifetime() { currentAwards=nullptr; }
Awards* Awards::instance() noexcept { return currentAwards; }
Awards::Awards(View* parent,ResourceManager& resources,Settings& settings,StringManager& strings,SoundManager& sound,
    InputBindings& input,ButtonInteraction& interaction,TooltipFrame::ScreenSize screenSize,ViewRegistry& views,ControlRegistry& controls)
    :AwardsLifetime(this),MenuView(parent,resources,settings,strings,views,controls),resources_(resources),settings_(settings),strings_(strings),sound_(sound),input_(input),interaction_(interaction),controls_(controls) {
    for(unsigned i=0;i<21;++i) buttons_[i]=new AwardButton(this,i,descriptions_[i],resources,settings,strings,sound,input,interaction,screenSize,views,controls);
    caption_=new TextLine(this,resources,settings,strings,views,controls);
    return_=new TextAnimationButton(this,resources,settings,strings,input,interaction,views,controls);return_->clickEvent=&returnClick;
    const auto xml=resources.loadSettingsXML(String("UI/Awards.xml"));loadXML(*xml);refresh();
}
Awards::~Awards()=default;
void Awards::loadXML(const XmlNode& node) {
    MenuView::loadXML(node);XmlNode *child=nullptr,*awards=nullptr;
    if(node.section(String("ReturnButton"),child)) return_->loadXML(*child);
    if(node.section(String("CaptionText"),child)) caption_->loadXML(*child);
    if(node.section(String("Awards"),awards)) for(unsigned i=0,count=std::min<unsigned>(21,unsigned(awards->children().size()));i<count;++i) {
        XmlNode* award=nullptr;if(!awards->section(i,award)) continue;
        if(award->section(String("DescText"),child)) descriptions_[i]=child->text();
        if(award->section(String("Button"),child)) buttons_[i]->loadXML(*child);
    }
}
void Awards::refresh() { for(unsigned i=0;i<21;++i) buttons_[i]->setOwned(profileProperties().booleans["awardShow"+std::to_string(i)]); }
void Awards::markOwned(unsigned index) { buttons_.at(index)->setOwned(1); }
AwardPanel* Awards::showAwardPanel(AwardEffect& effect,void(*)()) {
    auto* panel=new AwardPanel(effect,&onAwardPanelOk,resources_,settings_,strings_,sound_,input_,interaction_,registry_,controls_);
    panel->hide();panel->showTransparently();pendingEffect=&effect;return panel;
}
void Awards::onShown() { View::onShown();refresh(); }
void Awards::returnClick(ButtonPrototype*) { instance()->returnFromScreen(); }
void Awards::returnFromScreen() { discardTransparently();if(returnEvent) returnEvent(); }
const String& Awards::conditionKey(unsigned index) {
    static const std::array<String,21> keys={String("Award00Grow100"),String("Award01Produce100"),String("Award02Grow1000"),String("Award03Produce1000"),
        String("Award04Make20Task"),String("Award05Make40Task"),String("Award06Earn1000perMinute"),String("Award07UpgradeAllFarmDepots"),
        String("Award08BuildAllManufactureDepots"),String("Award09UpgradeAllManufactureDepots"),String("Award10Destroy100Depots"),
        String("Award11GrowAllAtOneTime"),String("Award12NoWaterNeedIn20Minute"),String("Award13Play30Minute"),String("Award14Play3Hour"),
        String("Award15SellItemPer40"),String("Award16Getting1000Gold"),String("Award17Getting10000Gold"),
        String("Award19MaxMarketAssortiment"),String("Award20Megaplanning"),String("Award21CollectAllAwards")};
    return keys.at(index);
}
void Awards::onAwardPanelOk(ButtonPrototype* button) {
    auto& awards=*instance();awards.registry_.requestDiscard(button->parent(),String(".\\Game\\Ui\\Awards.cpp102"));
    // Native dereferences the pending effect before its later null check.
    profileProperties().booleans[std::string(conditionKey(awards.pendingEffect->index()).view())]=true;
    if(awards.pendingEffect) {awards.pendingEffect->setPhase(4);awards.pendingEffect=nullptr;}
    awards.disable();
}
}
