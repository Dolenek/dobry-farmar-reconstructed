#pragma once
#include "Control.h"
#include <vector>
namespace farmar::recovered {
class ActionArea;class AnimableAppearance;
struct PersonTask {
    ActionArea* area=nullptr;
    Picture* marker=nullptr;
    Point2 target{};
};
struct PersonTasksContext {
    ResourceManager& resources;ViewRegistry& views;ControlRegistry& controls;
    const AnimableAppearance& markerAppearance;const Point2& markerSize;
};
// The original Person's +2828 queue and +2838 current record. Movement/actions
// consume these records; this component implements the actual queue and marks.
class PersonTasks final {
public:
    explicit PersonTasks(PersonTasksContext& c):context_(c) {}
    void add(ActionArea&,const Point2&); // 0044d2f0
    void finishCurrent(); // 0044d790
    void removeQueued(ActionArea&); // 0044dd80; leaves current and clickIndex alone
    PersonTask* next(); // 0044e300; finishes previous even when the queue is empty
    void clear(); // 0044f870
    bool empty() const noexcept {return queued_.empty();} // 00449930, excludes current
    const std::vector<PersonTask>& queued() const noexcept {return queued_;}
    const PersonTask& current() const noexcept {return current_;}
    static Point2 clickTarget(const ActionArea&,unsigned); // 00449a30
private:
    PersonTasksContext& context_;std::vector<PersonTask> queued_;PersonTask current_;
};
}
