#pragma once
#include "PersonVisuals.h"
namespace farmar::recovered {
class ActionArea;class StateView;
struct PersonWalkContext {
    FieldObject& actor;PersonVisuals& visual;ProfileProperties& profile;StateView& farm;
    const bool& cancelRequested;
};
// Motion state of the original ordinary walking action. Its arrival dispatcher
// is a separate actor/action responsibility, not replaced by an empty finish.
class PersonWalk final {
public:
    PersonWalk(ActionArea& area,PersonWalkContext& c):area_(area),context_(c) {}
    void begin(); // 004520a0
    bool step(); // 00452630, true hands control to the arrival dispatcher
    const Point2& goal() const noexcept {return goal_;}
    const Point2& velocity() const noexcept {return velocity_;}
    const Point2& relativeGoal() const noexcept {return relativeGoal_;}
    float remaining() const noexcept {return remaining_;}
    bool nearAtStart() const noexcept {return nearAtStart_;}
private:
    ActionArea& area_;PersonWalkContext& context_;
    Point2 goal_{},velocity_{},relativeGoal_{};float remaining_=0;bool nearAtStart_=false;
};
void storePersonPosition(FieldObject&,ProfileProperties&,const Point2&,bool move); // 0044e780
}
