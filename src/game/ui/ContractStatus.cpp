#include "ContractStatus.h"
#include "ProfileProperties.h"
#include "GameConfiguration.h"
#include "ResourceManager.h"
#include "XmlDocument.h"
#include "StringConversion.h"
#include "SoundManager.h"
#include "Sound.h"
#include "OriginalArithmetic.h"
#include <algorithm>
#include <cstring>
namespace farmar::recovered {
namespace {
struct GoalButton final:ExtendedAnimationButton {
    ContractStatus& owner;
    GoalButton(ContractStatus& status,View* parent,ResourceManager& resources,Settings& settings,StringManager& strings,
        InputBindings& input,ButtonInteraction& interaction,ContractStatusWorld& world,ViewRegistry& views,ControlRegistry& controls)
        :ExtendedAnimationButton(parent,resources,settings,strings,input,interaction,[&world]{return world.tutorialActive();},views,controls),owner(status) {}
    static void clickedEvent(ButtonPrototype* button) {static_cast<GoalButton*>(button)->owner.goalClicked();}
};
float seconds(const GameplayTime& time) {return float(originalUnsigned((time.hours*60U+time.minutes)*60U+time.seconds));}
std::int32_t signedBits(unsigned value) {std::int32_t result;std::memcpy(&result,&value,4);return result;}
}
ContractStatus::ContractStatus(View* parent,unsigned identifier,ResourceManager& resources,Settings& settings,StringManager& strings,
    InputBindings& input,ButtonInteraction& interaction,ProfileProperties& profile,GameConfiguration& configuration,
    SoundManager& audio,ContractStatusWorld& world,ViewRegistry& views,ControlRegistry& controls)
    :View(parent,views),id(identifier),resources_(resources),profile_(profile),configuration_(configuration),sound_(audio),world_(world) {
    substrate_=new Picture(this,resources,views,controls);
    goal_=new GoalButton(*this,substrate_,resources,settings,strings,input,interaction,world,views,controls);
    for(auto& icon:icons_) icon=new Picture(substrate_,resources,views,controls);
    for(auto& text:texts_) text=new TextLine(substrate_,resources,settings,strings,views,controls);
    time_=new TextLine(substrate_,resources,settings,strings,views,controls);
    progress_=new ProgressBar(substrate_,resources,views,controls);
    for(auto& check:checks_) check=new CheckBox(substrate_,resources,input,interaction,views,controls);
    const auto xml=resources.loadSettingsXML(String("ui/gameui/ContractStatus.xml"));loadXML(*xml);
    goal_->clickEvent=&GoalButton::clickedEvent;
}
ContractStatus::~ContractStatus() {if(lossSound_) lossSound_->releaseReference();if(winSound_) winSound_->releaseReference();}
std::string ContractStatus::key(const char* prefix) const {return std::string(prefix)+std::to_string(id);}
unsigned ContractStatus::stock(unsigned product) {return product<19 ? profile_.uints["WGoodCount"+std::to_string(product)]:0;}
bool ContractStatus::passCancel() {return profile_.booleans[key("taskPassCancel")];}
void ContractStatus::setPassCancel(bool value) {profile_.booleans[key("taskPassCancel")]=value;}
void ContractStatus::loadXML(const XmlNode& node) {
    View::loadXML(node);XmlNode* child=nullptr;
    const std::pair<const char*,View*> parts[]={{"Substrate",substrate_},{"GoalButton",goal_},
        {"ProductIcon01",icons_[0]},{"ProductIcon02",icons_[1]},{"Text01",texts_[0]},{"Text02",texts_[1]},
        {"TimeRemaindText",time_},{"Progress",progress_},{"Checked01",checks_[0]},{"Checked02",checks_[1]}};
    for(const auto& part:parts) if(node.section(String(part.first),child)) part.second->loadXML(*child);
    const auto los=node.attribute(WString(L"LosSound")),win=node.attribute(WString(L"WinSound"));
    // Native XML overwrites retained sound pointers without releasing the old reference.
    if(!los.empty()) lossSound_=resources_.getSound(toNarrow(los));
    if(!win.empty()) winSound_=resources_.getSound(toNarrow(win));
}
WString ContractStatus::formatTime(const GameplayTime& value,bool omitHours) {
    std::wstring text;
    const auto component=[&](unsigned number) {if(number<10) text+=L'0';text+=std::to_wstring(number);};
    if(!omitHours) {component(value.hours);text+=L':';}component(value.minutes);text+=L':';component(value.seconds);
    return WString(text);
}
void ContractStatus::updateDeadline(bool stepText) {
    const float left=seconds(remaining);
    progress_->current=float(originalMultiply(originalDivide(left,fullDuration),100));progress_->updateLid();
    time_->setString(formatTime(remaining,true));if(stepText) time_->enabledStep();
}
void ContractStatus::assign(GameplayEvent& value,bool restored) {
    event=&value;dirty=true;expired=false;if(!restored) setPassCancel(false);
    auto& contract=std::get<ContractEvent>(value.data);
    for(unsigned n=0;n<2;++n) if(contract.requirements[n].reqType==0) initialStocks[n]=stock(contract.requirements[n].productType);
    enable();show();accumulator=0;fullDuration=seconds(contract.winTime);
    if(!restored) {remaining=GameplayTime::fromSeconds(fullDuration);profile_.floats[key("taskTime")]=fullDuration;profile_.ints[key("task")]=signedBits(value.index);}
    else remaining=GameplayTime::fromSeconds(profile_.floats[key("taskTime")]);
    active=true;updateDeadline(true);refreshRequirements();
}
void ContractStatus::refreshRequirements() {
    const auto& contract=std::get<ContractEvent>(event->data);unsigned display=0;
    const auto hidePair=[&](unsigned n) {icons_[n]->hide();texts_[n]->hide();checks_[n]->hide();};
    for(const auto& requirement:contract.requirements) {
        if(requirement.reqType!=0) {hidePair(display ? 1:0);continue;}
        const unsigned have=(std::min)(stock(requirement.productType),requirement.count),left=requirement.count-have;
        auto& icon=*icons_[display];icon.copyAppearance(configuration_.products.at(requirement.productType).icon);icon.markTransformDirty();icon.updateTransform();icon.show();
        if(left==0) {texts_[display]->hide();checks_[display]->show();}
        else {texts_[display]->show();checks_[display]->hide();texts_[display]->setString(WString(std::to_wstring(signedBits(left))));texts_[display]->enabledStep();}
        ++display;
    }
    // Native trailing loop selects the pair by the fixed compact count, not
    // its loop counter: with no product requirements it hides pair 0 twice.
    for(unsigned n=display;n<2;++n) hidePair(display ? 1:0);
}
void ContractStatus::enabledStep() {if(world_.tutorialCanShow(0x12)) world_.showTutorial(0x12,*goal_);View::enabledStep();}
void ContractStatus::clearProfileTask() {
    profile_.ints[key("task")]=-1;for(unsigned n=0;n<2;++n) profile_.ints[key("req")+"_"+std::to_string(n)]=0;
}
void ContractStatus::fail() {
    active=false;disable();hide();clearProfileTask();const auto& contract=std::get<ContractEvent>(event->data);
    // Original +44 is Bonus, even on this loss path. Do not substitute +48 Penalty.
    world_.setMoney(signedBits(unsigned(profile_.ints["Money"])+unsigned(contract.bonus)));
    for(const auto& req:contract.requirements) if(req.reqType==0) world_.changeStock(req.productType,req.count,true,false);
    ++profile_.uints["awQTask"];world_.removeStatus(*this);
    // Original uses WinSound on the loss path and LosSound on expiry/passCancel.
    if(winSound_) sound_.clickSound(winSound_);
    if(!contract.winNote.empty()) world_.notify(contract.winNote,true);
}
void ContractStatus::absoluteStep() {
    if(active) {
        if(expired) {
            if(!passCancel()) fail();
            else {active=false;disable();hide();clearProfileTask();if(lossSound_) sound_.clickSound(lossSound_);world_.removeStatus(*this);
                const auto& note=std::get<ContractEvent>(event->data).lossNote;if(!note.empty()) world_.notify(note,true);
                profile_.uints["awQTask"]=0;world_.openResultDialog(*this);}
        }
        // The native method continues this frame after a completed/failed task.
        float elapsed=world_.elapsed();bool advancing=!world_.tutorialActive();if(!advancing) elapsed=0;
        if(world_.worldTransitioning()) {elapsed=0;advancing=false;}
        const double accumulated=originalAdd(elapsed,accumulator);accumulator=float(accumulated);
        if(accumulated>=1) {
            accumulator=float(originalSubtract(accumulated,1));
            if(remaining.seconds) --remaining.seconds;
            else if(remaining.minutes) {remaining.seconds=59;--remaining.minutes;}
            else if(remaining.hours) {remaining.minutes=59;--remaining.hours;}
            const float left=seconds(remaining);profile_.floats[key("taskTime")]=left;updateDeadline(false);
            if(left==0) {dirty=true;expired=true;}
        }
        if(dirty && advancing) {
            dirty=false;bool ready=true;
            for(const auto& req:std::get<ContractEvent>(event->data).requirements) if(req.reqType==0 && stock(req.productType)<req.count) {ready=false;break;}
            setPassCancel(true);if(ready) world_.openReadyDialog(*this);
        }
    }
    View::absoluteStep();
}
void ContractStatus::goalClicked() {
    if(world_.tutorialActive() && !world_.tutorialStepActive(0x12)) return;
    bool ready=true;for(const auto& req:std::get<ContractEvent>(event->data).requirements)
        if(req.reqType==0 && stock(req.productType)<req.count) ready=false;
    if(ready) world_.openReadyDialog(*this);else world_.openRequirementsDialog(*this);
}
}
