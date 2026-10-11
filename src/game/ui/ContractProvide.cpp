#include "ContractProvide.h"
#include "ContractStatus.h"
#include "ResourceManager.h"
#include "XmlDocument.h"
#include <cstring>
#include <stdexcept>
namespace farmar::recovered {
namespace {
ContractProvide* currentProvide=nullptr;
void loadChild(View& child,const XmlNode& node,const char* key) {XmlNode* section=nullptr;if(node.section(String(key),section)) child.loadXML(*section);}
int signedBits(unsigned value) {int result;std::memcpy(&result,&value,4);return result;}
}
ContractProvide* ContractProvide::instance() noexcept {return currentProvide;}
ContractProvide::ContractProvide(View* parent,ContractDialogContext& context,ContractOfferWorld& world)
    :ContractDialogBase(parent,context),world_(world) {
    if(currentProvide) throw std::runtime_error("Object already exist.");currentProvide=this;auto& c=context_;
    yes_=new TextAnimationButton(this,c.resources,c.settings,c.strings,c.input,c.interaction,c.views,c.controls);
    wait_=new TextAnimationButton(this,c.resources,c.settings,c.strings,c.input,c.interaction,c.views,c.controls);
    timeLabel_=new TextLine(this,c.resources,c.settings,c.strings,c.views,c.controls);
    timeValue_=new TextLine(this,c.resources,c.settings,c.strings,c.views,c.controls);
    yes_->clickEvent=&accepted;wait_->clickEvent=&waited;
    auto xml=c.resources.loadSettingsXML(String("Gameplay/ContractProvide.xml"));loadXML(*xml);
}
ContractProvide::~ContractProvide() {currentProvide=nullptr;}
void ContractProvide::accepted(ButtonPrototype*) {instance()->accept();}
void ContractProvide::waited(ButtonPrototype*) {instance()->wait();}
void ContractProvide::loadXML(const XmlNode& node) {
    ContractDialogBase::loadXML(node);loadChild(*yes_,node,"YesButton");loadChild(*wait_,node,"WaitButton");loadChild(*timeLabel_,node,"TimeText");loadChild(*timeValue_,node,"TimeTextValue");
}
void ContractProvide::enabledStep() {
    View::enabledStep();
    if((!yes_->state().enabled || !yes_->state().parentEnabled) && world_.tutorialCanShow(0x2a)) world_.showTutorial(0x2a,*yes_);
}
void ContractProvide::start(ContractOfferLetter& letter) {
    letter_=&letter;auto& event=letter.contractEvent();auto& contract=std::get<ContractEvent>(event.data);
    note_->setString(event.note,false);note_->enabledStep();
    for(unsigned n=0;n<2;++n) lines_[n].setup(contract.requirements[n],context_.configuration,0,false);
    bonusMoney_->setString(WString(std::to_wstring(contract.bonus)));penaltyMoney_->setString(WString(std::to_wstring(contract.penalty)));
    timeValue_->setString(ContractStatus::formatTime(contract.winTime,true));
    const auto money=world_.money();
    if(world_.activeContracts()==3 || money<contract.penalty) yes_->disable();else yes_->enable();
    suspendGame();
}
void ContractProvide::accept() {
    hideTransparently();resumeGame();world_.consumeLetter(*letter_);
    // Native re-reads the event after Farm consumes the actor; keep that order.
    auto& event=letter_->contractEvent();const auto& contract=std::get<ContractEvent>(event.data);
    const auto money=world_.money();const auto penalty=contract.penalty;
    world_.setMoney(signedBits(unsigned(money)-unsigned(penalty)));world_.activateContract(event);
}
void ContractProvide::wait() {hideTransparently();resumeGame();}
}
