#include "FarmField.h"
#include "GameConfiguration.h"
#include "GameRandom.h"
#include "MowBuilding.h"
#include "StateView.h"
#include "TutorialManager.h"
#include "OriginalArithmetic.h"
#include <cstring>
#include <stdexcept>
namespace farmar::recovered {
namespace {
int signedFrame(unsigned value) {int result;std::memcpy(&result,&value,4);return result;}
bool farmShown(FarmField& f) {const auto& s=f.context().world.farmScene().state();return s.shown && s.parentShown;}
bool farmStationary(FarmField& f) {return f.context().world.farmScene().movement==0;}
float tick() {if(!uiElapsedTime()) throw std::logic_error("Field actions require original elapsed timer");return uiElapsedTime()();}
float creation(FarmField& f) {return f.context().configuration.products.at(f.culture()).creationTime;}
bool harvested(FarmField& f) {if(f.culture()==0) {f.harvested=false;return false;}return f.harvested;}
class GrowingAction final:public GameAction {
public:
    GrowingAction(FarmField& f,float initial):GameAction(&f),field(f),initialProgress(initial) {} // 004361b0
    FarmField& field;float initialProgress;float unspecifiedTime;float halfTime;bool firstFlag=false,secondFlag=false;float speed;
    void begin() override { // 0043cc80
        field.auxiliary=0;field.setActionState(0);field.setProgress(initialProgress);
        halfTime=float(originalMultiply(originalSubtract(creation(field),field.animation().length),0.5));
        speed=float(originalAdd(originalMultiply(field.context().random.next(),6.103703537707217e-06f),0.9f));
    }
    void updateAnimation() { // 0043d5c0
        float advanced=float(originalSubtract(creation(field),field.progress()));field.setFrame(0);const auto& info=field.cultureInfo();const float lengths[]={info.firstTime,info.secondTime,info.lastTime};
        for(unsigned i=0;i<3;++i) {
            if(lengths[i]<=advanced) {field.advanceAnimation(lengths[i]);advanced=float(originalSubtract(advanced,lengths[i]));}
            else field.advanceAnimation(advanced);
            if(advanced<halfTime) break;advanced=float(originalSubtract(advanced,halfTime));
        }
    }
    bool step() override { // 0043f6a0
        if(field.actionState()==3) return true;float dt=tick();auto& c=field.context();if(c.profile.booleans["TFastSpeed"]) dt=float(originalMultiply(dt,10));dt=float(originalMultiply(dt,speed));
        float remaining=field.progress();const auto kind=field.culture();const auto& info=field.cultureInfo();bool reverse=false;
        if(kind==0) {
            if(field.water()==0) {const auto frame=signedFrame(field.frame()),stopped=signedFrame(info.notGrowFrame);if(frame==stopped) return false;if(frame>stopped) reverse=true;}
        }else {
            if(harvested(field)) return true;unsigned mode=0;const bool missing=field.fertility()==0 || field.water()==0;
            if(missing) {const auto frame=signedFrame(field.frame()),stopped=signedFrame(info.notGrowFrame);mode=frame==stopped ? 2:frame<stopped ? 0:1;}
            if(field.water()==0 && farmStationary(field) && farmShown(field) && c.world.tutorial().canShow(24)) c.world.tutorial().showArrow(24,&field);
            if(mode==2) return false;reverse=missing && mode==1;
        }
        if(reverse) {
            if(field.frame()!=info.notGrowFrame) {field.rewindAnimation(dt);const auto length=field.animation().length;
                field.setProgress(float(originalSubtract(creation(field),originalMultiply(originalDivide(creation(field),length),field.animation().time))));}
            return false;
        }
        remaining=float(originalSubtract(remaining,dt));if(remaining<0) remaining=0;field.setProgress(remaining);updateAnimation();return remaining<=0;
    }
    void finish() override { // 0043d0b0
        const auto kind=field.culture();if(field.actionState()==3) {field.installDead(float(field.randomDeathTime()));return;}
        if(kind!=0 && (field.water()==0 || harvested(field))) {field.harvested=false;field.playDeath();field.installPloughed();return;}
        field.playReady();field.installReady();
    }
};
class ReadyAction final:public GameAction {
public:
    explicit ReadyAction(FarmField& f):GameAction(&f),field(f) {} // 004361e0
    FarmField& field;float unusedElapsed=0;
    void begin() override { // 0043fd80
        field.auxiliary=0;field.setActionState(1);field.showReadySign(true);auto& c=field.context();auto& bits=c.profile.ints["awGrowType"];const unsigned next=unsigned(bits)|(1U<<(field.culture()&31));std::memcpy(&bits,&next,4);
        auto& t=c.world.tutorial();if(field.culture()==0 && t.canShow(3) && !t.canShow(2)) {
            if(c.profile.uints["PCarry"]==6) t.complete(3);
            else if(farmStationary(field) && farmShown(field)) {auto* mower=MowBuilding::instance();if(!mower) throw std::logic_error("Field tutorial3 requires actual MowBuilding");t.showAction(3,mower);}
        }
    }
    bool step() override { // 0043da40
        if(field.auxiliary==2 || field.actionState()==3 || harvested(field)) return true;
        const auto kind=field.culture();if(field.water()==0) return true;if(kind!=0) return false;
        auto& t=field.context().world.tutorial();if(t.canShow(4) && farmStationary(field) && farmShown(field) && !t.canShow(3)) t.showAction(4,&field);return false;
    }
    void finish() override { // 0043dee0
        field.showReadySign(false);
        if(field.auxiliary==2) {field.auxiliary=0;field.setFrame(field.beginGrowFrame);
            if(field.culture()!=0 && field.water()==0) {field.installPloughed();return;}field.installGrowing(creation(field));return;}
        if(field.actionState()==3) {field.installDead(float(field.randomDeathTime()));return;}
        if(field.water()==0) {field.installGrowing(creation(field));return;}
        field.harvested=false;field.playDeath();field.installPloughed();
    }
};
class DeadAction final:public GameAction {
public:
    DeadAction(FarmField& f,float time):GameAction(&f),field(f),remaining(time) {} // 00436180
    FarmField& field;float remaining;
    void begin() override {field.setActionState(3);field.setProgress(remaining);field.setFrame(0);} // 0043c440
    bool step() override { // 0043f180
        float dt=tick();auto& c=field.context();auto& t=c.world.tutorial();
        if(farmStationary(field) && farmShown(field)) {
            if(!t.canShow(22)) {if(t.canShow(23)) t.showAction(23,&field);}
            else if(!t.canShow(21)) {auto packages=c.world.seedPackages();if(!packages.empty()) t.showAction(22,packages.front());}
        }
        if((t.isProcessed(21) || t.isProcessed(22)) && !t.isProcessed(23)) dt=0;
        if(field.culture()==0) {remaining=float(originalSubtract(remaining,dt));if(remaining<0) remaining=0;field.setProgress(remaining);if(remaining!=0) return false;}
        return true;
    }
    void finish() override { // 0043c830
        if(field.culture()==0xffffffffU) field.setCulture(0);field.installGrowing(creation(field));
    }
};
class PloughedAction final:public GameAction {
public:
    explicit PloughedAction(FarmField& f):GameAction(&f),field(f) {} // 00436210
    FarmField& field;float unusedElapsed=0,speed;
    void begin() override { // 0043e550
        field.auxiliary=0;field.setActionState(2);field.setProgress(0);speed=float(originalDivide(field.animation().length,field.context().configuration.products.at(field.culture()).dieTime));
    }
    bool step() override { // 0043e960
        if(field.frame()==0) return true;const float dt=tick();field.rewindAnimation(originalMultiply(dt,speed));return false;
    }
    void finish() override {field.setCulture(0);field.installDead(float(field.randomDeathTime()));} // 0043ed60
};
}
void FarmField::installGrowing(float progress) {actions_.set(std::make_unique<GrowingAction>(*this,progress));}
void FarmField::installReady() {actions_.set(std::make_unique<ReadyAction>(*this));}
void FarmField::installDead(float time) {actions_.set(std::make_unique<DeadAction>(*this,time));}
void FarmField::installPloughed() {actions_.set(std::make_unique<PloughedAction>(*this));}
}
