#pragma once
#include "PersonWalk.h"
namespace farmar::recovered {
struct PersonVehicleWalkContext {PersonWalkContext& motion;PersonVisualWorld& world;};
// Motion body of the 76-byte native action; arrival dispatcher is separate.
class PersonVehicleWalk final {
public:
    PersonVehicleWalk(ActionArea& area,PersonVehicleWalkContext& c):area_(area),context_(c) {}
    void begin(); // 00452bc0
    bool step(); // 00453380
    const Point2& goal() const noexcept {return goal_;}
    const Point2& velocity() const noexcept {return velocity_;}
    const Point2& relativeGoal() const noexcept {return relativeGoal_;}
    const Point2& xVelocity() const noexcept {return xVelocity_;}
    const Point2& yVelocity() const noexcept {return yVelocity_;}
    float remaining() const noexcept {return remaining_;}
    float xRemaining() const noexcept {return xRemaining_;}
    float yRemaining() const noexcept {return yRemaining_;}
    bool nearAtStart() const noexcept {return near_;}
    bool direct() const noexcept {return direct_;}
private:
    ActionArea& area_;PersonVehicleWalkContext& context_;
    Point2 goal_{},velocity_{},relativeGoal_{},xVelocity_{},yVelocity_{};
    float remaining_=0,xRemaining_=0,yRemaining_=0;bool near_=false,direct_=false;
    void snap();void advance(const Point2&,float);
};
}
