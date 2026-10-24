#include "Person.h"
#include "FarmField.h"
#include "FarmAreas.h"
#include "SeedPackage.h"
#include "DigBuilding.h"
#include "MowBuilding.h"
#include "WaterSource.h"
#include "Warehouse.h"
#include "ElasticAnimation.h"
#include "WaterEmitters.h"
#include "TutorialManager.h"
#include "StateView.h"
#include "OriginalArithmetic.h"
#include <stdexcept>
namespace farmar::recovered {
namespace {
template<class T> T& actual(T* value) {if(!value) throw std::logic_error("Person action requires actual Farm dependency");return *value;}
void refreshTools() {actual(WaterSource::instance()).refreshAppearance();actual(DigBuilding::instance()).refreshAppearance();actual(MowBuilding::instance()).refreshAppearance();}
Point2 position(const Control& c) {return {c.geometry().x,c.geometry().y};}
class IdleAction final:public GameAction {
    Person& person;
public:
    explicit IdleAction(Person& p):GameAction(nullptr),person(p) {}
    void begin() override { //00451760
        auto& v=person.visual();const auto held=person.heldType();
        if(held==10) {v.waterUse().hide();v.showDiscrete(1,actual(person.carriedSeed).count(),4);}
        else {
            if(held==1) {v.waterUse().current=float(originalMultiply(originalDivide(person.heldWater(),float(actual(WaterSource::instance()).capacity())),100));v.waterUse().updateLid();v.waterUse().show();}
            else v.waterUse().hide();
            if(held==6) {auto& mow=actual(MowBuilding::instance());v.showDiscrete(1,v.carriedProducts(),mow.levels.at(unsigned(mow.level())).productionNum);}
            else v.showDiscrete(0,0,0);
        }
        v.playIdle();
    }
    bool step() override { //0045a2c0
        if(person.cancelRequested) person.requestCancel(false);if(!person.tasks().empty()) return true;
        person.context().profile.booleans["awAdecvStart"]=false;const float dt=uiElapsedTime()();person.advanceAnimation(dt);return false;
    }
    void finish() override { //00451c30
        const auto held=person.heldType();auto* task=person.tasks().next();if(!task || !task->area) throw std::logic_error("Person idle finish requires queued task");
        person.installWalk(*task->area,held==6 || (held==2 && actual(DigBuilding::instance()).level()!=0));
    }
};
class WalkingAction final:public GameAction {
    Person& person;ActionArea& area;bool vehicle;PersonWalk walk;PersonVehicleWalk drive;
public:
    WalkingAction(Person& p,ActionArea& a,bool v):GameAction(nullptr),person(p),area(a),vehicle(v),walk(a,p.walkContext()),drive(a,p.vehicleContext()) {}
    void begin() override {if(vehicle) drive.begin();else walk.begin();}
    bool step() override {return vehicle ? drive.step():walk.step();}
    void finish() override {person.arrive(area);} //0045a730 /0045bbe0
};
class HoseIdleAction final:public GameAction {
    Person& person;PersonTask* task=nullptr;
public:
    explicit HoseIdleAction(Person& p):GameAction(nullptr),person(p) {}
    void begin() override { //004590f0
        person.setHeldType(11);person.setInvertX(false);person.visual().waterUse().hide();person.visual().showDiscrete(0,0,1);person.visual().playIdle();
        auto& water=actual(WaterSource::instance());const Point2 at{float(originalSubtract(water.geometry().x,40)),float(originalAdd(water.geometry().y,35))};person.setPosition(at);
        const auto origin=position(person.context().farm.movingPicture());person.storePosition({float(originalSubtract(at[0],origin[0])),float(originalSubtract(at[1],origin[1]))},false);
    }
    bool step() override { //0045c5a0; reads elapsed even though no motion is made
        uiElapsedTime()();if(person.cancelRequested) person.requestCancel(false);if(person.tasks().empty()) return false;task=person.tasks().next();
        if(!task || !task->area) throw std::logic_error("Person hose idle requires actual queued area");
        if(task->area->areaType==64) return false;if(task->area->areaType==4) {person.tasks().finishCurrent();return false;}return true;
    }
    void finish() override { //00459590
        auto* area=task->area;if(!area) throw std::logic_error("Person hose finish lost task area");
        if(area->areaType==1) {auto* field=dynamic_cast<FarmField*>(area);if(!field) throw std::logic_error("Person hose requires actual Field");
            // Native constructs work before completing the marker, but installs
            // it (and begins emission) only after marker completion.
            installHoseField(*field);
        } else {person.setHeldType(0);person.installWalk(*area,false);}
    }
private:void installHoseField(FarmField&);
};
class WorkingAction final:public GameAction {
    Person& person;PersonActionKind job;ActionArea& area;SeedPackage* seed;float remaining=0,rate=0;
    FarmField& field() {auto* f=dynamic_cast<FarmField*>(&area);if(!f) throw std::logic_error("Person work requires actual Field");return *f;}
public:
    WorkingAction(Person& p,PersonActionKind kind,ActionArea& a,SeedPackage* package):GameAction(nullptr),person(p),job(kind),area(a),seed(package) {}
    void begin() override {
        auto& v=person.visual();switch(job) {
        case PersonActionKind::Dig: {auto& dig=actual(DigBuilding::instance());remaining=dig.levels.at(unsigned(dig.level())).digTime;v.playJob(0);person.sounds().play(dig.level()>0 ? 1:0);break;} //00453b10
        case PersonActionKind::TakeSpade:remaining=v.actionTimes[6];break; //00454750
        case PersonActionKind::TakeScythe:remaining=v.actionTimes[7];break; //00454fd0
        case PersonActionKind::TakeBucket:remaining=v.actionTimes[8];person.sounds().play(5);break; //00455850
        case PersonActionKind::Sow:remaining=v.actionTimes[0];v.playJob(8);person.sounds().play(3);break; //00456470
        case PersonActionKind::Harvest: {auto& mow=actual(MowBuilding::instance());remaining=mow.levels.at(unsigned(mow.level())).mowTime;v.playJob(4);person.sounds().play(4);break;} //004570b0
        case PersonActionKind::Store:remaining=v.actionTimes[2];person.sounds().play(7);break; //004578b0
        case PersonActionKind::Water:remaining=v.actionTimes[1];v.playJob(10);person.sounds().play(2);break; //004584d0
        case PersonActionKind::HoseWater: { //00459a20 /0042f5a0
            person.setHeldType(11);const Point2 target{field().geometry().x,float(originalAdd(field().geometry().y,10))};v.playHose(target);
            const auto& offset=v.waterEmitterOffsets.at(v.hoseFrame);const Point2 from{float(originalAdd(person.geometry().x,offset[0])),float(originalAdd(person.geometry().y,offset[1]))};
            auto& emitter=person.context().world.hoseEmitter();emitter.angle=v.waterAngles.at(v.hoseFrame);emitter.start(0);emitter.setPosition(from);
            emitter.startPoint={float(originalSubtract(from[0],emitter.geometry().x)),float(originalSubtract(from[1],emitter.geometry().y))};
            // Native0042f5a0 uses from for both local points; target is unused.
            emitter.targetPoint=emitter.startPoint;emitter.emission.generating=true;rate=float(actual(WaterSource::instance()).capacity());person.sounds().play(6);break;}
        default:throw std::logic_error("Invalid Person timed work kind");
        }
    }
    bool step() override {
        const float dt=uiElapsedTime()();if(job==PersonActionKind::HoseWater) {if(person.cancelRequested) person.requestCancel(false);field().addWater(float(originalMultiply(dt,rate)));return field().water()==field().waterCapacity;} //0045c9c0
        person.advanceAnimation(dt);const auto left=originalSubtract(remaining,dt);remaining=float(left);return !(left>0); //453f20 and seven matching timed bodies: compare unspilled value, unordered completes
    }
    void finish() override {
        switch(job) {
        case PersonActionKind::Dig:field().setCulture(0);field().setActionState(3);field().setFertility(field().fertilCapacity);person.sounds().stopShovel();person.sounds().stopTractor();break; //00454320
        case PersonActionKind::TakeSpade:person.setHeldType(2);refreshTools();break; //00454ba0
        case PersonActionKind::TakeScythe:person.setHeldType(6);refreshTools();break; //00455420
        case PersonActionKind::TakeBucket:person.setHeldType(1);person.setHeldWater(float(actual(WaterSource::instance()).capacity()));refreshTools();break; //00456020
        case PersonActionKind::Sow: {auto& package=actual(seed);field().auxiliary=1;field().setCulture(package.kind());const auto count=package.count()-1;package.setCount(count);
            if(!count) {person.tasks().removeQueued(package);person.context().world.removeSeed(package);person.setHeldType(0);}break;} //00456c50, cached carriedSeed pointer is deliberately retained
        case PersonActionKind::Harvest: {field().auxiliary=2;field().setActionState(0);person.sounds().stopScythe();++person.context().profile.uints["awQCollect"];
            const auto kind=field().culture();person.setProductCount(kind,person.productCount(kind)+1);person.visual().activity=12;
            if(!person.context().world.areas().hasReadyCulture(kind)) {auto& mask=person.context().profile.ints["awGrowType"];mask=static_cast<int>(unsigned(mask)&~(1U<<(kind&31)));}break;} //0045bfa0
        case PersonActionKind::Store: {if(person.visual().carriedProducts()) for(unsigned i=0;i<8;++i) {const auto count=person.productCount(i);if(count) {const auto accepted=actual(Warehouse::instance()).give(i,count,true,false);person.setProductCount(i,count-accepted);if(count-accepted) break;}}break;} //00458080
        case PersonActionKind::Water:person.setHeldWater(float(field().addWater(person.heldWater())));person.sounds().stopWatering();break; //00458cb0
        case PersonActionKind::HoseWater:person.context().world.hoseEmitter().emission.generating=false;person.sounds().stop(6);person.installIdle(true);return; //00459eb0
        default:throw std::logic_error("Invalid Person work completion");
        }
        person.installIdle();
    }
};
void HoseIdleAction::installHoseField(FarmField& field) {auto action=std::make_unique<WorkingAction>(person,PersonActionKind::HoseWater,field,nullptr);person.tasks().finishCurrent();person.actions().set(std::move(action));}
}
void Person::installIdle(bool hose) {if(hose) actions().set(std::make_unique<HoseIdleAction>(*this));else actions().set(std::make_unique<IdleAction>(*this));}
void Person::installWalk(ActionArea& area,bool vehicle) {actions().set(std::make_unique<WalkingAction>(*this,area,vehicle));}
void Person::installWork(PersonActionKind kind,ActionArea& area,SeedPackage* seed) {actions().set(std::make_unique<WorkingAction>(*this,kind,area,seed));}
void Person::recordActionStarted(bool started) {
    auto& p=context_.profile;if(!started) {p.booleans["awAdecvStart"]=false;return;}
    if(!p.booleans["awAdecvStart"]) {if(tasks_.queued().size()<30) return;p.booleans["awAdecvStart"]=true;p.uints["awAdecvClick"]=1;}
    else ++p.uints["awAdecvClick"];
}
void Person::arrive(ActionArea& area) { //0045a730;0045bbe0 is a direct delegate
    tasks_.finishCurrent();if(cancelRequested) {requestCancel(false);installIdle();return;}
    if(area.areaType==32 && static_cast<SeedPackage&>(area).status()!=0) {installIdle();return;}
    auto& tips=actual(TutorialManager::instance());auto& farm=context_.farm;
    const auto blocked=[&](std::initializer_list<int> steps) {for(int step:steps) if(tips.activeStep(step)) return true;return false;};
    const auto farmReady=[&] {return !farm.movement && farm.state().shown && farm.state().parentShown;};
    const auto idle=[&] {installIdle();recordActionStarted(false);};
    const auto work=[&](PersonActionKind kind,SeedPackage* seed=nullptr) {installWork(kind,area,seed);recordActionStarted(true);};
    const auto fly=[&](Control& to,int kind,View* reveal=nullptr) {actual(ElasticAnimation::instance()).begin(position(*this),position(to),kind,reveal);};
    const auto dropSeed=[&] {auto& seed=actual(carriedSeed);seed.setStatus(0);carriedSeed=nullptr;seed.hideTips();fly(seed,9,&seed);};
    const auto returnHeld=[&](int held) {if(held==6) {auto& mow=actual(MowBuilding::instance());fly(mow,mow.level()+5);}else if(held==2) {auto& dig=actual(DigBuilding::instance());fly(dig,dig.level()+1);}else if(held==1) {auto& water=actual(WaterSource::instance());if(water.level()==0) fly(water,0);}else if(held==10) dropSeed();};
    const auto held=heldType();switch(area.areaType) {
    case 1: {auto& field=static_cast<FarmField&>(area);
        if(held==2) {if(field.actionState()==3) {idle();return;}const int money=context_.world.money();const int cost=static_cast<int>(field.digCost);
            if(cost>money) {idle();return;}installWork(PersonActionKind::Dig,field);context_.world.setMoney(static_cast<int>(unsigned(money)-unsigned(cost)));recordActionStarted(true);return;}
        if(held==10) {if(field.actionState()==3) work(PersonActionKind::Sow,carriedSeed);else idle();return;}
        if(held==6) {
            if(field.actionState()!=1) {idle();return;}bool can=productCapacity();if(can && actual(MowBuilding::instance()).level()!=3 && carriesOtherProducts(field.culture())) {
                can=false;if(farmReady() && tips.repeatedMixedAttempt() && !tips.activeStep(44)) tips.showAction(44,&field);
            }
            if(can) {work(PersonActionKind::Harvest);return;}installIdle();if(farmReady() && tips.canShow(40)) tips.showAction(40,&actual(Warehouse::instance()));recordActionStarted(false);return;
        }
        if(held==1 && heldWater()!=0) {work(PersonActionKind::Water);return;}idle();return;}
    case 2: {
        if(held!=6) {idle();return;}auto& warehouse=actual(Warehouse::instance());const auto free=warehouse.freeSpace();const auto products=visual_.carriedProducts();
        if(free<products && farmReady() && tips.repeatedCapacityAttempt() && !tips.activeStep(41)) tips.showAction(41,&warehouse);
        if(!visual_.carriedProducts() || !warehouse.freeSpace()) {idle();return;}
        if(!tips.active() || blocked({41,40,5,44})) work(PersonActionKind::Store);else idle();return;}
    case 16:
        if(blocked({4,3,1,5,2,22,23}) || held==2) {idle();return;}
        if(held==6) {installWork(PersonActionKind::TakeSpade,area);auto& mow=actual(MowBuilding::instance());fly(mow,mow.level()+5);recordActionStarted(true);return;}
        if(held==10) dropSeed();else if(held==1) returnHeld(held);work(PersonActionKind::TakeSpade);return;
    case 8:
        if(blocked({1,5,2,20,21,22,23}) || held==6) {idle();return;}
        if(held==10 || held==1 || held==2) returnHeld(held);work(PersonActionKind::TakeScythe);return;
    case 4: {
        if(blocked({4,3,5,20,21,22,23})) {idle();return;}
        auto& water=actual(WaterSource::instance());if(held==1) {const auto capacity=float(water.capacity());bool filled=false;if(capacity!=heldWater()) {setHeldWater(float(water.capacity()));filled=true;sounds_.play(5);}installIdle();recordActionStarted(filled);return;}
        if(held!=0 && held!=6 && held!=2 && held!=10) {idle();return;}returnHeld(held);
        if(water.level()==1 || water.level()==2) installIdle(true);else if(water.level()==0) installWork(PersonActionKind::TakeBucket,area);else throw "impossible";
        recordActionStarted(true);return;}
    case 32:
        if(blocked({20,21})) {idle();return;}returnHeld(held);static_cast<SeedPackage&>(area).setStatus(3);setHeldType(10);carriedSeed=static_cast<SeedPackage*>(&area);installIdle();recordActionStarted(true);return;
    default:idle();return;
    }
}
}
