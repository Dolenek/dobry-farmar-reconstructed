#include "SalesHistory.h"
#include "ProfileProperties.h"
#include "OriginalArithmetic.h"
#include <iterator>
#include <stdexcept>
namespace farmar::recovered {
void SalesHistory::prune(float time) {
    cutoff_=float(originalSubtract(time,60.f));
    auto next=records_.begin();
    while(next!=records_.end()) {
        if(!(next->time<cutoff_)) return; // equality and unordered both stop
        total_-=next->amount;
        const auto successor=records_.erase(next);
        // 0040c337 loads [successor], including the sentinel's next link.
        // Preserve this actual walk; it can leave alternating expired records.
        next=successor==records_.end() ? records_.begin():std::next(successor);
    }
}
void SalesHistory::record(std::uint32_t amount,float time) {
    prune(time);
    if(records_.size()==0x1fffffffU) throw std::length_error("list<T> too long");
    records_.push_back({time,amount});total_+=amount;
    if(total_>999) profile_.booleans["awEarn"]=true;
}
}
