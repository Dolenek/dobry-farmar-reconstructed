#include "Upgrade.h"
#include "GameConfiguration.h"
#include "ProfileProperties.h"
#include "SoundManager.h"
#include "Sound.h"
#include "Channel.h"
#include "XmlDocument.h"
#include "View.h"
#include <algorithm>
#include <cstddef>
#include <cstring>
#include <string>
namespace farmar::recovered {
namespace {
std::int32_t signedBits(std::uint32_t value) noexcept {std::int32_t output;std::memcpy(&output,&value,4);return output;}
std::int32_t increment(std::int32_t value) noexcept {return signedBits(static_cast<std::uint32_t>(value)+1U);}
std::int32_t decrement(std::int32_t value) noexcept {return signedBits(static_cast<std::uint32_t>(value)-1U);}
}
ViewUpgradeAliasWords viewUpgradeAliasWords(const View& view) noexcept {
    // Original cView+24/+28 correspond to these six state bytes and two
    // unspecified padding bytes. Retain this allocation's object words;
    // neither a fixed key nor the reference debugger's BAAD fill is inserted.
    static_assert(sizeof(ViewState)==20 && offsetof(ViewState,shown)==12 && offsetof(ViewState,absoluteEnabled)==16);
    ViewUpgradeAliasWords result;
    std::memcpy(&result.propertyId,&view.state().shown,4);
    std::memcpy(&result.levelCount,&view.state().absoluteEnabled,4);
    return result;
}
void refreshViewUpgradeAlias(ProfileProperties& profile,View& view) {
    const auto words=viewUpgradeAliasWords(view);
    const auto key="Upgrade"+std::to_string(words.propertyId);
    const auto previous=profile.ints[key]; // original 00417050 inserts first
    auto level=previous;
    if(level>=words.levelCount) level=decrement(words.levelCount);
    auto& value=profile.ints[key]; // 004174b0 reads it again before writing
    if(value!=level) value=level;
    (void)view.type(); // raw View vtable+4, not the real Upgrade::refresh
}
UpgradeContext::UpgradeContext(GameConfiguration& config,ProfileProperties& values,SoundManager& sound)
    :configuration(config),profile(values),sound_(sound) {}
UpgradeContext::~UpgradeContext()=default;
void UpgradeContext::clearSound() {channel_.reset();}
void UpgradeContext::playSound() {
    channel_.reset();channel_.reset(sound_.playSound(static_cast<cSound*>(configuration.upgradeSound()),false,true));
}
Upgrade::Upgrade(UpgradeContext& context,std::uint32_t type,std::uint32_t id):itemType(type),propertyId(id),context_(context) {}
Upgrade::~Upgrade()=default;
void Upgrade::loadXML(const XmlNode& node) {
    XmlNode* upgrades=nullptr;if(!node.section(String("Upgrades"),upgrades)) return;
    const auto count=std::min<std::size_t>(upgrades->children().size(),4);
    for(std::uint32_t i=0;i<count;++i) {
        XmlNode* definition=nullptr;if(upgrades->section(String("Up"+std::to_string(i)),definition)) {
            loadLevel(i,*definition);levelCount=increment(levelCount);
        }
    }
}
std::int32_t Upgrade::level() {return context_.profile.ints["Upgrade"+std::to_string(propertyId)];}
bool Upgrade::canUpgrade(bool checkMoney,bool checkAvailability) {
    const auto next=increment(level());
    if(checkAvailability && signedBits(context_.profile.uints["UpAvail"+std::to_string(itemType)])<next) return false;
    if(checkMoney) {
        const auto cost=signedBits(context_.configuration.upgradeCost(itemType,static_cast<std::uint32_t>(next)));
        return context_.profile.ints["Money"]>=cost;
    }
    return true;
}
void Upgrade::setLevel(std::int32_t value,bool sound) {
    if(value>=levelCount) value=decrement(levelCount);
    if(level()!=value) {context_.profile.ints["Upgrade"+std::to_string(propertyId)]=value;if(sound) context_.playSound();}
    refresh();
}
}
