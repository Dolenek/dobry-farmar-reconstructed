#include "PersonVehicleWalk.h"
#include "ActionArea.h"
#include "SeedPackage.h"
#include "StateView.h"
#include "OriginalArithmetic.h"
#include <cmath>
namespace farmar::recovered {
void PersonVehicleWalk::begin() {
    auto& c=context_.motion;near_=false;const auto carry=c.profile.uints["PCarry"];
    // 00452c2b reads this level even though its return value is unused.
    if(carry==2) context_.world.digLevel();else if(carry==6) context_.world.mowLevel();
    auto spot=c.visual.hotSpot();const Point2 origin{c.actor.geometry().x,c.actor.geometry().y};const auto goal=area_.goal(carry);
    if(!(std::fabs(originalSubtract(goal[0],spot[0]))>2) && !(std::fabs(originalSubtract(goal[1],spot[1]))>2)) {near_=true;return;}
    direct_=false;
    if(!(std::fabs(originalSubtract(spot[0],goal[0]))<10)) {
        if(!(std::fabs(originalSubtract(spot[1],goal[1]))<10)) {
            const bool down=!(spot[1]>goal[1]);c.visual.playAxisWalk({0,down ? 100.f:-100.f});c.visual.hotSpotOffset(down ? 4:0);
            const float speed=c.visual.speed;const auto offset=c.visual.rawHotSpot(2);const auto inverse=originalDivide(1,speed);
            if(!(origin[0]>goal[0])) {xRemaining_=float(originalMultiply(originalSubtract(originalSubtract(goal[0],origin[0]),-offset[0]),inverse));xVelocity_={speed,0};}
            else {xRemaining_=float(originalMultiply(originalAdd(originalSubtract(origin[0],goal[0]),offset[0]),inverse));xVelocity_={-speed,0};}
            if(!(origin[1]>goal[1])) {yRemaining_=float(originalMultiply(originalSubtract(originalSubtract(goal[1],origin[1]),offset[1]),inverse));yVelocity_={0,speed};}
            else {yRemaining_=float(originalMultiply(originalAdd(originalSubtract(origin[1],goal[1]),offset[1]),inverse));yVelocity_={0,-speed};}
        }else {direct_=true;c.visual.playAxisWalk({!(spot[0]>goal[0]) ? 100.f:-100.f,0});spot=c.visual.hotSpot();}
    }else {direct_=true;c.actor.setInvertX(false);c.visual.playAxisWalk({0,!(spot[1]>goal[1]) ? 100.f:-100.f});spot=c.visual.hotSpot();}
    if(direct_) {
        // X remains unspilled, Y is a float at 00452d83. Duration is stored
        // without pop before taking its reciprocal, then inverse is a float.
        const auto x=originalSubtract(goal[0],spot[0]);const float y=float(originalSubtract(goal[1],spot[1]));
        const auto duration=originalDivide(originalSqrt(originalAdd(originalMultiply(x,x),originalMultiply(y,y))),c.visual.speed);
        remaining_=float(duration);const float inverse=float(originalDivide(1,duration));velocity_={float(originalMultiply(x,inverse)),float(originalMultiply(y,inverse))};
    }
    goal_=goal;const auto& moving=c.farm.movingPicture().geometry();relativeGoal_={float(originalSubtract(goal[0],moving.x)),float(originalSubtract(goal[1],moving.y))};
}
void PersonVehicleWalk::advance(const Point2& position,float dt) {
    auto& c=context_.motion;c.actor.setPosition(position);const auto& moving=c.farm.movingPicture().geometry();
    storePersonPosition(c.actor,c.profile,{float(originalSubtract(position[0],moving.x)),float(originalSubtract(position[1],moving.y))},false);
    c.actor.setTime(float(originalAdd(dt,c.actor.animation().time)));
}
void PersonVehicleWalk::snap() {
    auto& c=context_.motion;const auto offset=c.visual.hotSpotOffset(c.visual.direction);const auto& moving=c.farm.movingPicture().geometry();
    // Native stores the Y world sum before subtraction; X stays unspilled.
    const float y=float(originalAdd(moving.y,relativeGoal_[1]));c.actor.setPosition({float(originalSubtract(originalAdd(moving.x,relativeGoal_[0]),offset[0])),float(originalSubtract(y,offset[1]))});
    storePersonPosition(c.actor,c.profile,{float(originalSubtract(relativeGoal_[0],offset[0])),float(originalSubtract(relativeGoal_[1],offset[1]))},false);
}
bool PersonVehicleWalk::step() {
    auto& c=context_.motion;if(c.cancelRequested || near_ || (area_.areaType==32 && static_cast<SeedPackage&>(area_).status()==2)) return true;
    float dt=uiElapsedTime()();
    if(direct_) {const auto left=originalSubtract(remaining_,dt);remaining_=float(left);if(left<=0) {snap();return true;}
        advance({float(originalAdd(originalMultiply(dt,velocity_[0]),c.actor.geometry().x)),float(originalAdd(c.actor.geometry().y,originalMultiply(dt,velocity_[1])))},dt);return false;}
    if(xRemaining_==0 && yRemaining_==0) {snap();return true;}
    Point2 position{c.actor.geometry().x,c.actor.geometry().y};
    if(yRemaining_!=0) {
        if(yRemaining_<dt) dt=yRemaining_;const auto left=originalSubtract(yRemaining_,dt);yRemaining_=float(left);
        position={float(originalAdd(originalMultiply(dt,yVelocity_[0]),position[0])),float(originalAdd(position[1],originalMultiply(dt,yVelocity_[1])))};
        // The zero test uses the still unspilled subtraction at 004536a6.
        if(left==0) c.visual.playAxisWalk(xVelocity_);
    }else if(xRemaining_!=0) {
        if(xRemaining_<dt) dt=xRemaining_;xRemaining_=float(originalSubtract(xRemaining_,dt));
        position={float(originalAdd(originalMultiply(dt,xVelocity_[0]),position[0])),float(originalAdd(position[1],originalMultiply(dt,xVelocity_[1])))};
    }
    advance(position,dt);return false;
}
}
