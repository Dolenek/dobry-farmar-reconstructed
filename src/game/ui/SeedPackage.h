#pragma once
#include "ActionArea.h"
#include "GameAction.h"
#include "ProgressDiscrete.h"
#include "GameConfiguration.h"
#include "ProfileProperties.h"
namespace farmar::recovered {
class StateView;class SeedPackage;class FarmField;
struct SeedPackageWorld:ActionAreaWorld {
    virtual StateView& farmScene()=0;
    virtual bool seedMoveActive()=0;virtual void setSeedMoveActive(bool)=0;
    virtual SeedPackage* selectedSeed()=0;virtual void setSelectedSeed(SeedPackage*)=0;
    virtual bool canPlaceSeed(SeedPackage&,const Point2&)=0; // 0042ce30
    virtual ActionArea* topAt(const Point2&,unsigned mask)=0; // 0042df60
    virtual std::vector<ActionArea*> areasAt(const Point2&,unsigned mask)=0; // 00431950
    virtual void setFarmActionButtonEnabled(bool)=0; // 0042c7e0 / 0042c7f0
    virtual int money()=0;virtual void setMoney(int)=0;
    virtual void removeSeed(SeedPackage&)=0; // 00433420
    virtual FarmField* fieldForSeedTutorial()=0; // 0042e7c0
    virtual Control& digBuilding()=0;
};
struct SeedPackageContext {ActionAreaContext area;GameConfiguration& configuration;ProfileProperties& profile;SeedPackageWorld& world;};
class SeedPackage final:public ActionArea {
public:
    SeedPackage(View*,unsigned kind,unsigned count,const Point2&,unsigned status,bool unpaid,SeedPackageContext&); // 0045e130
    ~SeedPackage() override; // 0045ce00
    void loadXML(const XmlNode&) override; // 0045d020
    void enabledStep() override; // 0045e340
    void absoluteStep() override {} // original RET004bd820
    void clicked() override;void unclicked() override; // 0045d660 / 0045dbe0
    void hovered() override;void unhovered() override; // 0045e8d0 / 0045e100
    void onStateChanged(ButtonState state) override {AnimationButton::onStateChanged(state);} // 0045ceb0
    unsigned status() const noexcept {return status_;} // 0045cea0
    void setStatus(unsigned); // 0045cf50
    unsigned kind() const noexcept {return kind_;}unsigned count() const noexcept {return count_;}
    void setCount(unsigned); // 0045cee0
    ProgressDiscrete& use() noexcept {return *use_;}
    ActionProcessor& actions() noexcept {return actions_;}
    Point2 pressPosition{},dragOffset{};Color normalColor{0,0,0,0},unfitColor{0,0,0,0};bool unpaid;
private:
    SeedPackageContext& seedContext_;ActionProcessor actions_;std::array<AnimableAppearance,8> unusedAppearances_;
    unsigned status_; // native +648 is unspecified before the first setStatus
    unsigned kind_,count_;ProgressDiscrete* use_;
    cResource *takeSound_=nullptr,*dropSound_=nullptr;
    Point2 dragPosition(const Point2&) const;void play(cResource*);
};
SeedPackage* createFarmSeed(unsigned,SeedPackageContext&); // 0042c960, actual BuySeeds callback
}
