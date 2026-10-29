#pragma once
#include "GameString.h"
#include <cstdint>
#include <memory>
namespace farmar::recovered {
class GameConfiguration;class ProfileProperties;class SoundManager;class cChannel;class XmlNode;class View;
// 0040be10 passes a focused View directly to the old cUpgrade methods.
// These words are View flags plus allocation padding, not its real Upgrade.
struct ViewUpgradeAliasWords {std::uint32_t propertyId;std::int32_t levelCount;};
ViewUpgradeAliasWords viewUpgradeAliasWords(const View&) noexcept;
void refreshViewUpgradeAlias(ProfileProperties&,View&);
// cGame's shared +a0c channel, used by every cUpgrade. The game owns the
// configuration/sound manager and clears this channel at its native teardown.
class UpgradeContext final {
public:
    UpgradeContext(GameConfiguration&,ProfileProperties&,SoundManager&);
    ~UpgradeContext();
    UpgradeContext(const UpgradeContext&)=delete;
    UpgradeContext& operator=(const UpgradeContext&)=delete;
    void playSound(); // 00404170: stop/delete current, then playSound(false,true)
    void clearSound();
    cChannel* channel() const noexcept {return channel_.get();}
    GameConfiguration& configuration;ProfileProperties& profile;
private:
    SoundManager& sound_;std::unique_ptr<cChannel> channel_;
};
// Original cUpgrade is abstract (005ef5a0 contains two _purecall slots).
// Derived farm/building classes supply real appearance/UI refresh and XML
// level loading. This base supplies the shared original property operations.
class Upgrade {
public:
    Upgrade(UpgradeContext&,std::uint32_t itemType,std::uint32_t propertyId); // 004169c0
    virtual ~Upgrade(); // 00416970
    Upgrade(const Upgrade&)=delete;
    Upgrade& operator=(const Upgrade&)=delete;
    void loadXML(const XmlNode&); // 00416a30, Up0..Up3 by section count; cumulative levelCount
    std::int32_t level(); // 00417050, Upgrade{propertyId}, inserts missing property
    bool canUpgrade(bool checkMoney,bool checkAvailability); // 00417430, signed comparisons
    void setLevel(std::int32_t,bool playSound); // 004174b0, upper clamp only, always refresh
    virtual void refresh()=0; // original vtable +4
    virtual void loadLevel(std::uint32_t,const XmlNode&)=0; // original vtable +8
    std::uint32_t itemType,propertyId;
    std::int32_t levelCount=0;
    String name; // original +8, ctor/destructor only in this base
protected:
    UpgradeContext& context_;
};
}
