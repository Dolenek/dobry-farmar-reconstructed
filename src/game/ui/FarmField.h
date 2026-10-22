#pragma once
#include "ActionArea.h"
#include "GameAction.h"
#include "ProfileProperties.h"
namespace farmar::recovered {
class FarmField;class StateView;class GameConfiguration;class GameRandom;
struct FarmFieldWorld:ActionAreaWorld {
    virtual StateView& farmScene()=0;
    virtual bool hasDryField()=0; // 0042caa0: any field water<=waterSignLevel
    virtual bool farmClickHeld()=0; // Farm +108, virtual +44
    virtual void setFarmClickHeld(bool)=0; // Farm virtual +48
    virtual bool areaGate(unsigned type)=0; // 0042d700 / 0042db30, no excluded area
    virtual ActionArea* firstAreaAt(const Point2&,unsigned mask)=0; // 0042e3a0
    virtual void indicateField(FarmField&)=0; // 0042c9e0
    virtual void clearFieldIndication(FarmField&)=0; // 0042c800, current owner only
    virtual void openFieldTip(FarmField&)=0; // FieldTip 004402a0, singleton 0065f774
    virtual bool fieldTipPresent()=0;
    virtual void hideFieldTip(FarmField&)=0; // 004402d0, only the current selected owner
    virtual std::vector<ActionArea*> seedPackages()=0; // 00432260, ActionArea type32
};
struct FarmFieldContext {ActionAreaContext area;ProfileProperties& profile;GameConfiguration& configuration;GameRandom& random;FarmFieldWorld& world;};
struct FieldAbsorption {Point2 water{},fertil{};};
struct FieldCultureInfo {
    unsigned firstFrames,secondFrames;float firstTime,secondTime,lastTime;
    unsigned notGrowFrame;float notGrowProgress;String mask;
    // Constructor initializes only mask in the native 56-byte record.
};
class FarmField final:public ActionArea {
public:
    FarmField(View*,unsigned group,unsigned index,int upgradeIndex,FarmFieldContext&); // 0043c120
    ~FarmField() override; // 00436340
    void loadXML(const XmlNode&) override; // 004388e0
    void enabledStep() override;void absoluteStep() override; // 0043ac90 / 0043b0d0
    void onEnabled() override;void onDisabled() override; // 00436280 / 004362c0
    void onStateChanged(ButtonState) override {} // actual RET 00436470
    void clicked() override;void unclicked() override; // 00438740 / 004387f0
    void hovered() override;void unhovered() override; // 00438620 / 004386d0
    unsigned culture();unsigned actionState();float progress();float water();float fertility();
    void setActionState(unsigned);void setProgress(float);void setWater(float);void setFertility(float);
    double addWater(float); // 00451680
    void setCulture(unsigned); // 0043b5f0
    void refreshAppearance(); // 00438200
    void refreshPurchase(int current,int available); // 0043ba70
    void preview(bool); // 00436880
    void showReadySign(bool); // 004364b0
    void playDeath();void playReady(); // 00436240 / 00436260
    double randomDeathTime(); // 0043c3e0, actual x87 result
    void installGrowing(float);void installReady();void installDead(float);void installPloughed();
    void advanceAnimation(double); // original actions directly add to +1ac and dirty it
    void rewindAnimation(double);
    FieldCultureInfo& cultureInfo();
    FarmFieldContext& context() noexcept {return fieldContext_;}
    ActionProcessor& actions() noexcept {return actions_;}
    Picture& waterSign() noexcept {return *waterSign_;}Picture& readySign() noexcept {return *readySign_;}
    Picture& fertilSign() noexcept {return *fertilSign_;}Picture& hiddenPicture() noexcept {return *hidden_;}
    unsigned id;int upgradeIndex;bool active=false;unsigned auxiliary; // +514 ctor unspecified
    AnimableAppearance hiddenAppearance,readyToBuy,underWater;
    std::array<AnimableAppearance,8> cultureAppearances;
    std::array<FieldCultureInfo,8> cultures;
    unsigned digCost=0,beginGrowFrame;float waterCapacity=0,fertilCapacity=0,waterSignLevel=0,fertilSignLevel=0;
    float waterAbsorption=0,fertilAbsorption=0,ploughedToGrass=0;
    std::array<FieldAbsorption,8> absorption;
    bool harvested=false; // native +944
private:
    FarmFieldContext& fieldContext_;Picture *waterSign_,*readySign_,*fertilSign_,*hidden_;
    cResource *deathSound_=nullptr,*readySound_=nullptr;ActionProcessor actions_;
    std::string key(const char*) const;float sample(const Point2&);
}; // native2384 / 950; primary vtable95, MI2
}
