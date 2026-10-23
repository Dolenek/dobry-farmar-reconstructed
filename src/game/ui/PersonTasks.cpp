#include "PersonTasks.h"
#include "ActionArea.h"
#include "OriginalArithmetic.h"
#include <stdexcept>
namespace farmar::recovered {
namespace {
float markerDepth(const ActionArea& area,unsigned index) {
    return float(originalAdd(originalAdd(originalUnsigned(index),area.deep()),100));
}
}
Point2 PersonTasks::clickTarget(const ActionArea& area,unsigned index) {
    if(area.clickOffsets.empty()) throw std::logic_error("Original Person marker reposition requires nonempty ActionArea offsets");
    const auto& offset=index<area.clickOffsets.size() ? area.clickOffsets[index]:area.clickOffsets.back();
    return {float(originalAdd(offset[0],area.geometry().x)),float(originalAdd(offset[1],area.geometry().y))};
}
void PersonTasks::add(ActionArea& area,const Point2& target) {
    auto* marker=new Picture(&area,context_.resources,context_.views,context_.controls);
    marker->copyAppearance(context_.markerAppearance);marker->setPosition(target);
    marker->setSize(context_.markerSize);marker->setDeep(markerDepth(area,area.clickIndex));marker->show();
    queued_.push_back(PersonTask{&area,marker,target});
}
void PersonTasks::finishCurrent() {
    auto* area=current_.area;if(!area) return;
    --area->clickIndex;unsigned index=0;
    for(auto& task:queued_) if(task.area==area) {
        task.marker->setPosition(clickTarget(*area,index));task.marker->setDeep(markerDepth(*area,index));++index;
    }
    current_.area=nullptr;
    context_.views.requestDiscard(current_.marker,String(".\\Game\\Farm\\Person.cpp536"));current_.marker=nullptr;
}
void PersonTasks::removeQueued(ActionArea& area) {
    for(auto it=queued_.begin();it!=queued_.end();) {
        if(it->area!=&area) {++it;continue;}
        context_.views.requestDiscard(it->marker,String(".\\Game\\Farm\\Person.cpp551"));it=queued_.erase(it);
    }
}
PersonTask* PersonTasks::next() {
    finishCurrent();if(queued_.empty()) return nullptr;
    current_=queued_.front();queued_.erase(queued_.begin());return &current_;
}
void PersonTasks::clear() {
    finishCurrent();for(auto& task:queued_) {
        context_.views.requestDiscard(task.marker,String(".\\Game\\Farm\\Person.cpp506"));--task.area->clickIndex;
    }
    std::vector<PersonTask>().swap(queued_);
}
}
