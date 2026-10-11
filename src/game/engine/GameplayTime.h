#pragma once
#include <cstdint>
#include "GameWString.h"
namespace farmar::recovered {
// cTime: three unsigned components. Contract deadlines, scheduled events and
// restored GameTime use this representation. This has no virtual/resource base.
struct GameplayTime {
    std::uint32_t hours=0,minutes=0,seconds=0;
    void loadXML(const WString& text); // 004b0b70, original colon checks
    static GameplayTime fromSeconds(float value) noexcept; // 004adac0
    bool operator<(const GameplayTime& other) const noexcept {
        if(hours!=other.hours) return hours<other.hours;
        if(minutes!=other.minutes) return minutes<other.minutes;
        return seconds<other.seconds;
    }
    bool operator==(const GameplayTime& other) const noexcept { return hours==other.hours && minutes==other.minutes && seconds==other.seconds; }
};
}
