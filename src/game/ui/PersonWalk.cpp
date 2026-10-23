#include "PersonWalk.h"
#include "ActionArea.h"
#include "SeedPackage.h"
#include "StateView.h"
#include "OriginalArithmetic.h"
#include <cmath>
#include <stdexcept>
namespace farmar::recovered {
void storePersonPosition(FieldObject& actor,ProfileProperties& profile,const Point2& position,bool move) {
    if(move) actor.FieldObject::setPosition(position);
    profile.floats["XPersPos"]=position[0];profile.floats["YPersPos"]=position[1];
}
void PersonWalk::begin() {
    auto& c=context_;nearAtStart_=false;goal_=area_.goal(c.profile.uints["PCarry"]);
    auto spot=c.visual.hotSpot();
    const auto close=[&] {return !(std::fabs(originalSubtract(goal_[0],spot[0]))>2) && !(std::fabs(originalSubtract(goal_[1],spot[1]))>2);};
    if(close()) {nearAtStart_=true;return;}
    const Point2 delta{float(originalSubtract(goal_[0],spot[0])),float(originalSubtract(goal_[1],spot[1]))};
    if(delta[0]!=0 || delta[1]!=0) c.visual.playWalk(delta);spot=c.visual.hotSpot();
    if(close()) {nearAtStart_=true;return;}
    const float x=float(originalSubtract(goal_[0],spot[0])),y=float(originalSubtract(goal_[1],spot[1]));
    const auto duration=originalDivide(originalSqrt(originalAdd(originalMultiply(x,x),originalMultiply(y,y))),c.visual.speed);
    // 0045225e stores duration without popping it; its reciprocal uses the
    // unspilled value, then the shared coefficient is stored as float.
    remaining_=float(duration);const float inverse=float(originalDivide(1,duration));
    velocity_={float(originalMultiply(x,inverse)),float(originalMultiply(y,inverse))};
    const auto& moving=c.farm.movingPicture().geometry();relativeGoal_={float(originalSubtract(goal_[0],moving.x)),float(originalSubtract(goal_[1],moving.y))};
}
bool PersonWalk::step() {
    auto& c=context_;if(c.cancelRequested) return true;if(nearAtStart_) {remaining_=0;return true;}
    if(area_.areaType==32 && static_cast<SeedPackage&>(area_).status()==2) return true;
    const float dt=uiElapsedTime()();const auto left=originalSubtract(remaining_,dt);remaining_=float(left);
    if(!(left<=0)) {
        const Point2 position{float(originalAdd(originalMultiply(dt,velocity_[0]),c.actor.geometry().x)),float(originalAdd(c.actor.geometry().y,originalMultiply(dt,velocity_[1])))};
        c.actor.FieldObject::setPosition(position);auto* parent=static_cast<Control*>(c.actor.parent());if(!parent) throw std::logic_error("Person walking requires actual moving-picture parent");
        storePersonPosition(c.actor,c.profile,{float(originalSubtract(position[0],parent->geometry().x)),float(originalSubtract(position[1],parent->geometry().y))},false);
        c.actor.setTime(float(originalAdd(dt,c.actor.animation().time)));return false;
    }
    // The original refreshes the world hotspot before fetching its mirrored
    // offset, then snaps relative to the current moving-picture coordinates.
    c.visual.hotSpot();const auto offset=c.visual.hotSpotOffset(c.visual.direction);const auto& moving=c.farm.movingPicture().geometry();
    c.actor.FieldObject::setPosition({float(originalSubtract(originalAdd(moving.x,relativeGoal_[0]),offset[0])),float(originalSubtract(originalAdd(moving.y,relativeGoal_[1]),offset[1]))});
    storePersonPosition(c.actor,c.profile,{float(originalSubtract(relativeGoal_[0],offset[0])),float(originalSubtract(relativeGoal_[1],offset[1]))},false);return true;
}
}
