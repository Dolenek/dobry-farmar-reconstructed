#pragma once
#include "TutorialTips.h"
#include "StateView.h"
#include "ProfileProperties.h"
#include <array>
#include <vector>
namespace farmar::recovered {
class GameMenu;
// Required primitive operations of the actual world. The manager implements
// the original predicates; providers must supply recovered objects and values.
struct TutorialWorld {
    virtual ~TutorialWorld()=default;
    virtual GameMenu& hud()=0;
    virtual StateView& farm()=0;
    virtual StateView& wareDetails()=0;
    virtual StateView& manufacture()=0;
    virtual View& selectionOverlay()=0;
    virtual View& sceneOverlay()=0;
    virtual Control& well()=0;
    virtual Control& warehouse()=0;
    virtual std::vector<Control*> farmPlots()=0; // 00431df0
    virtual std::vector<Control*> farmPlants()=0; // 00432260
    virtual std::vector<Control*> wareItems()=0; // 00498400
    virtual double plotWater(Control&)=0; // 00437d90, original x87 result
    virtual float plotWaterLimit(Control&) const=0; // +8a8
    virtual int plotKind(Control&)=0; // 00436d90
    virtual int plantState(Control&)=0; // 0045cea0
    virtual int wareItemUpgrade(Control&)=0; // +364 -> 00417050
    virtual unsigned wareItemFlags(Control&) const=0; // +574
    virtual int anchorState(Control&) const=0; // +364
    virtual unsigned parentProductCount(Control&)=0; // 0049f010, uints ProdCount
    virtual int wellUpgradeLevel()=0; // 00417050
    virtual int mowerUpgradeLevel()=0; // 00417050
    virtual double wellCapacity()=0; // 004683a0, original x87 result
    virtual int characterHeldType()=0; // 0044ec00
    virtual float characterHeldWater()=0; // 0044f120, original float spill
    virtual int characterActivity()=0; // character virtual +170, read each call
    virtual bool characterToolFlag()=0; // 0044edd0
    virtual unsigned warehouseProductCount(unsigned)=0; // 00467fa0
    virtual unsigned manufactureProductCount(unsigned)=0; // 0046eb70
    virtual unsigned warehouseFree()=0; // 00465e70
    virtual bool seedMoveActive() const=0; // 0065f721
    virtual bool wareSelectionPresent() const=0; // 00660150, 00499c80
    virtual bool selectionPresent() const=0; // 0065ff64 +248
    virtual bool sceneSelectionPresent() const=0; // 00660010 +248
    virtual View& contractConfirm()=0; // 0065f76c
    virtual View& contractAim()=0; // 0065f768
    virtual View& contractDialog764()=0; // 0065f764
    virtual View& contractProvide()=0; // 0065f760
};
struct TutorialManagerContext {
    ResourceManager& resources;Settings& settings;StringManager& strings;InputBindings& input;ButtonInteraction& interaction;
    ProfileProperties& profile;TutorialWorld& world;ViewRegistry& views;ControlRegistry& controls;
};
struct TutorialStep {
    int index=-1;WString text;Point2 offset{},arrowOffset{};
    float multiplier=-1,interlinear=-1;
    TutorialActionTip::Predicate predicate=nullptr; // native unspecified unless installed
    void loadXML(unsigned,const XmlNode&); // 004cdfd0, native 60-byte record
};
class TutorialManager;
class TutorialManagerLifetime {
protected:explicit TutorialManagerLifetime(TutorialManager*);~TutorialManagerLifetime();
};
class TutorialManager final:private TutorialManagerLifetime,public View,public TutorialTipHost {
public:
    explicit TutorialManager(TutorialManagerContext&); // 004cfce0, singleton 0065f73c
    ~TutorialManager() override; // 004d0490, discard only four; repeated tip retained
    void loadXML(const XmlNode&) override; // 004ce160
    void enabledStep() override; // 004d1160
    bool active() const noexcept {return currentStep!=-1;} // 004cdee0
    bool activeStep(int value) const noexcept {return currentStep==value;} // 004cdef0
    void requestCancel() noexcept {cancelRequested=true;} // 004cdf10
    bool isProcessed(unsigned); // 004d0990, disabled -> false without profile access
    bool canShow(unsigned); // 004d0a20
    void complete(unsigned); // 004d0ac0
    void showCommon(unsigned); // 004ce650
    void showArrow(unsigned,Control*); // 004ceb00
    void showDisabledAnchor(unsigned,Control*); // 004cef60
    void showAction(unsigned,Control*); // 004cf3c0
    void showRepeatedAction(unsigned,Control*); // 004cf7d0
    void finishTip(TutorialCommonTip&) override; // 004d0c30
    bool takeCancelRequest() override;
    void setGameplayPaused(bool) override;
    bool evaluate(unsigned,Control*); // original 29 installed predicates
    static TutorialManager* instance() noexcept;
    static std::string profileKey(unsigned); // 004cfbe0
    TutorialCommonTip& common() noexcept {return *common_;}
    TutorialArrowTip& arrow() noexcept {return *arrow_;}
    TutorialDisabledAnchorTip& disabledAnchor() noexcept {return *disabled_;}
    TutorialActionTip& action() noexcept {return *action_;}
    TutorialRepeatedActionTip& repeatedAction() noexcept {return *repeated_;}
    std::array<TutorialStep,50> steps;
    int currentStep=-1,justProcessed=-1; // justProcessed native unspecified until complete
    bool resumeSelection=false,resumeOverlay=false,resumeHUD=false,resumeHUDabsolute=false;
    bool cancelRequested=false,tutorialEnabled=true;
    float elapsed=0,valueC10=-1,valueC18=-1;unsigned valueC14=100;
private:
    TutorialManagerContext& context_;TutorialTipContext tipContext_;
    TutorialCommonTip* common_;TutorialArrowTip* arrow_;TutorialDisabledAnchorTip* disabled_;
    TutorialActionTip* action_;TutorialRepeatedActionTip* repeated_;
    void captureHUD();void configure(TutorialCommonTip&,const TutorialStep&);void restoreHUD();
};
}
