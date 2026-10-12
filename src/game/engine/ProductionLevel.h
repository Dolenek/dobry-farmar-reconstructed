#pragma once
#include "AnimableAppearance.h"
#include "GameString.h"
#include <cstdint>
namespace farmar::recovered {
class Control;class XmlNode;class ResourceManager;
// Original 116-byte record: count +0, time +4, appearances +8/+30, String +58.
// 0049a120 initializes only appearances/String. Valid shipped UpN XML assigns
// both numeric fields; they deliberately have no invented constructor default.
struct ProductionLevel {
    std::uint32_t productionCount;
    float maximumProductionTime;
    AnimableAppearance appearance,hovered;
    String maskName;
    void loadXML(const XmlNode&,ResourceManager&); // 0049cd60, reached through cUpgrade +8
};
}
