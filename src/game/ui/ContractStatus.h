#pragma once
#include "View.h"
#include "GameplayEvents.h"
#include "ExtendedTextButton.h"
#include "TextLine.h"
#include "CheckBox.h"
#include "ProgressBar.h"
namespace farmar::recovered {
class ProfileProperties;class GameConfiguration;class SoundManager;class cSound;
class ContractStatus;
// Required bindings to the actual tutorial/world/dialog controllers. No default
// implementation: constructing this component alone does not supply a game.
struct ContractStatusWorld {
    virtual ~ContractStatusWorld()=default;
    virtual bool tutorialActive() const=0; // 004cdee0
    virtual bool tutorialStepActive(unsigned) const=0; // 004cdef0
    virtual bool tutorialCanShow(unsigned) const=0; // 004d0a20
    virtual void showTutorial(unsigned,View&)=0; // 004cf3c0
    virtual bool worldTransitioning() const=0; // Farm/DepotDetails/Manufacture +4c
    virtual float elapsed() const=0; // timer +8
    virtual void setMoney(std::int32_t)=0; // 00410d60, includes world refresh
    virtual void changeStock(unsigned,unsigned,bool,bool)=0; // 004682d0
    virtual void removeStatus(ContractStatus&)=0; // 004c06b0
    virtual void notify(const WString&,bool)=0; // 004c44a0
    virtual void openReadyDialog(ContractStatus&)=0; // 004b7fe0
    virtual void openRequirementsDialog(ContractStatus&)=0; // 004a94e0
    virtual void openResultDialog(ContractStatus&)=0; // 004b6b90
};
class ContractStatus final:public View {
public:
    ContractStatus(View*,unsigned,ResourceManager&,Settings&,StringManager&,InputBindings&,
        ButtonInteraction&,ProfileProperties&,GameConfiguration&,SoundManager&,ContractStatusWorld&,
        ViewRegistry& views=globalViewRegistry(),ControlRegistry& controls=globalControlRegistry()); // 004bb830
    ~ContractStatus() override; // 004b92c0
    void loadXML(const XmlNode&) override; // 004ba730
    void enabledStep() override; // 004b98e0
    void absoluteStep() override; // 004bcc20
    void assign(GameplayEvent&,bool restored); // 004bd2f0
    void refreshRequirements(); // 004ba0b0
    void fail(); // 004bbee0
    bool passCancel(); // 004bc460, inserts default profile boolean
    void setPassCancel(bool); // 004bc840
    void goalClicked(); // 004b9350
    static WString formatTime(const GameplayTime&,bool omitHours); // 004b1080
    unsigned stock(unsigned product); // 00467fa0
    unsigned id;
    float accumulator=0,fullDuration=0;
    GameplayTime remaining;
    bool active=false,dirty=false,expired=false;
    std::array<unsigned,2> initialStocks{}; // unused requirement slots originally unspecified
    GameplayEvent* event=nullptr; // borrowed, assigned by Gameplay/Farm
    Picture& substrate() noexcept {return *substrate_;}
    ExtendedAnimationButton& goalButton() noexcept {return *goal_;}
    Picture& productIcon(unsigned n) {return *icons_.at(n);}
    TextLine& requirementText(unsigned n) {return *texts_.at(n);}
    CheckBox& requirementCheck(unsigned n) {return *checks_.at(n);}
    TextLine& timeText() noexcept {return *time_;}
    ProgressBar& progress() noexcept {return *progress_;}
    cSound* lossSound() const noexcept {return lossSound_;}
    cSound* winSound() const noexcept {return winSound_;}
private:
    ResourceManager& resources_;ProfileProperties& profile_;GameConfiguration& configuration_;
    SoundManager& sound_;ContractStatusWorld& world_;
    Picture* substrate_;ExtendedAnimationButton* goal_;
    std::array<Picture*,2> icons_;std::array<TextLine*,2> texts_;std::array<CheckBox*,2> checks_;
    TextLine* time_;ProgressBar* progress_;cSound* lossSound_=nullptr; cSound* winSound_=nullptr;
    std::string key(const char*) const;
    void clearProfileTask();void updateDeadline(bool stepText);
};
}
