#pragma once
#include "GameplayTime.h"
#include <array>
#include <cstdint>
#include <memory>
#include <variant>
#include <vector>
namespace farmar::recovered {
class XmlNode;
enum class GameplayEventType : std::uint32_t { DepotUpgrade=0,SeedAvailable=1,Money=2,DestroyField=3,DestroyDepot=4,Contract=5,Popularity=6,FieldUpgrade=7,Trigger=8,End=9 };
struct DepotUpgradeEvent { std::uint32_t depotType=0xffffffffU,upgrade=0xffffffffU; };
struct SeedAvailableEvent { std::uint32_t seedType=0xffffffffU; };
struct RandomValueEvent { RandomValueEvent():randomFactor(0) {} float randomFactor;std::int32_t value; };
struct ContractRequirement {
    std::uint32_t productType,count,reqType;
    ContractRequirement():reqType(1) {} // 004b0a80 does not initialize the first two
};
struct ContractEvent {
    ContractEvent() {} // initialize members only, preserve unspecified requirement scalars
    GameplayTime winTime{},letterTime{};
    std::int32_t bonus=0,penalty=0,initialBonus=0,initialPenalty=0;
    float randomFactor=0;
    std::array<ContractRequirement,2> requirements,initialRequirements;
    WString winNote,lossNote;
    std::uint32_t state=0,unused0=0,unused1=0;
};
struct PopularityValue { std::uint32_t productType;std::int32_t value; };
struct PopularityEvent { std::vector<PopularityValue> values,initialValues; };
struct FieldUpgradeEvent { FieldUpgradeEvent() {} std::int32_t upgradeIndex; }; // original ctor leaves this unspecified
struct TriggerEvent { TriggerEvent() {} std::int32_t money,wareLevel; }; // loader always defaults WareLevel to -1
struct EndEvent {};
using GameplayEventData=std::variant<DepotUpgradeEvent,SeedAvailableEvent,RandomValueEvent,ContractEvent,PopularityEvent,FieldUpgradeEvent,TriggerEvent,EndEvent>;
class GameplayEvent final {
public:
    explicit GameplayEvent(GameplayEventType type); // 004b09e0 and derived constructors
    void loadXML(const XmlNode& node); // original per-type virtual loader
    GameplayEventType type;
    GameplayTime time;
    WString note;
    float range=0;
    std::uint32_t index; // assigned only after timeline sorting, original +34
    GameplayEventData data;
};
// Data ownership/sorting part of the actual Gameplay constructor. Execution,
// randomization, profile restoration and world dispatch are separate recovery
// work; this class supplies no replacement callbacks or simulated mechanics.
class GameplayEventDefinitions final {
public:
    void loadXML(const XmlNode& node); // 004b50d0, append/insertion, no implicit clear
    const std::vector<std::unique_ptr<GameplayEvent>>& events() const noexcept { return events_; }
    GameplayTime endTime;
    bool noTimedEvents=true;
    std::uint32_t endEventIndex; // native +3c only assigned when Type9 is found
private:
    std::vector<std::unique_ptr<GameplayEvent>> events_;
};
}
