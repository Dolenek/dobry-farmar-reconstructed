#include "GameAction.h"
#include <utility>
namespace farmar::recovered {
namespace {unsigned actionCount=0;}
GameAction::GameAction(View* target,unsigned kind):owner(target),type(kind) {++actionCount;}
GameAction::~GameAction() {--actionCount;}
unsigned GameAction::count() noexcept {return actionCount;}
void ActionProcessor::set(std::unique_ptr<GameAction> action) {
    // Native deletes the previous object before assigning and beginning the
    // replacement, including set(nullptr). It never calls old.finish here.
    current_.reset();current_=std::move(action);if(current_) current_->begin();
}
void ActionProcessor::step() {
    if(current_ && current_->step()) {
        auto completed=std::move(current_);completed->finish();
        // Destroy only the detached completed action. A replacement created
        // by finish remains active, as in native 004153b0.
    }
}
}
