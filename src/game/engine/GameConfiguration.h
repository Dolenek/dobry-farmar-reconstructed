#pragma once
#include "AnimableAppearance.h"
#include "GameWString.h"
#include <array>
#include <cstdint>
namespace farmar::recovered {
class cResource;
class ResourceManager;
class XmlNode;
struct ProductionInfo {
    // Native 100 bytes: 00403610 initializes only the two appearances.
    float dieTime;
    std::int32_t priceRadius,goodAttPriceRadius;
    float maxSellFrequency;
    AnimableAppearance icon,iconLarge;
    float creationTime;
    void loadXML(const XmlNode& node,ResourceManager& resources); // 0040af00
};
struct SeedInfo {
    // Native 44 bytes: 004036b0 initializes only the appearance.
    std::uint32_t price;
    AnimableAppearance icon;
    void loadXML(const XmlNode& node,ResourceManager& resources); // 0040b560
};
// Configuration owned by cGame 00413210. The complete cGame constructor and
// world are not supplied by this component. cGame must first load View XML.
class GameConfiguration final {
public:
    explicit GameConfiguration(ResourceManager& resources):resources_(resources) {}
    ~GameConfiguration(); // sound release first, 00410830
    GameConfiguration(const GameConfiguration&)=delete;
    GameConfiguration& operator=(const GameConfiguration&)=delete;
    void loadXML(const XmlNode& node); // configuration portion of 0040e6e0
    std::array<ProductionInfo,19> products;
    std::array<SeedInfo,8> seeds;
    std::array<std::array<std::uint32_t,4>,15> costs;
    std::array<WString,5> upgradeStrings;
    cResource* upgradeSound() const noexcept { return upgradeSound_; }
private:
    ResourceManager& resources_;
    cResource* upgradeSound_=nullptr; // cGame +a08
};
}
