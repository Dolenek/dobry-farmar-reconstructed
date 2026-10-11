#include "Upgrade.h"
namespace {
ProfileProperties* upgradeNativeProfile=nullptr;
void* __fastcall upgradeProperty(void* storage,void*,const String* key) {
    const std::string name(key->view());
    if(storage==image+0x25ff20) return &upgradeNativeProfile->ints[name];
    require(storage==image+0x25ff44,"Upgrade property store differs");return &upgradeNativeProfile->uints[name];
}
std::ostringstream upgradeKey;
void __fastcall upgradeStreamConstruct(void*,void*,void*,int) {upgradeKey.str("");upgradeKey.clear();}
void* __cdecl upgradeWriteLiteral(void* self,const char* text) {upgradeKey<<text;return self;}
void* __fastcall upgradeWriteIndex(void* self,void*,unsigned value) {upgradeKey<<value;return self;}
String* __fastcall upgradeStreamContents(void*,void*,String* output) {return new(output) String(upgradeKey.str());}
using UpgradeLoads=std::vector<std::pair<unsigned,const XmlNode*>>;
using UpgradeRefreshes=std::vector<std::pair<int,int>>;
UpgradeLoads upgradeNativeLoads;UpgradeRefreshes upgradeNativeRefreshes;
bool upgradeMutateCount=false;
void __fastcall upgradeLoadLevel(void* self,void*,unsigned index,XmlAdapter* node) {
    upgradeNativeLoads.emplace_back(index,node->node);if(upgradeMutateCount) field<int>(self,0x28)=-4;
}
void __fastcall upgradeRefresh(void* self,void*) {
    const auto key="Upgrade"+std::to_string(field<unsigned>(self,0x24));
    upgradeNativeRefreshes.emplace_back(upgradeNativeProfile->ints[key],field<int>(self,0x28));
}
struct UpgradeTrace final:Upgrade {
    UpgradeLoads loads;UpgradeRefreshes refreshes;
    UpgradeTrace(UpgradeContext& context,unsigned type,unsigned id):Upgrade(context,type,id) {}
    void loadLevel(unsigned index,const XmlNode& node) override {loads.emplace_back(index,&node);if(upgradeMutateCount) levelCount=-4;}
    void refresh() override {refreshes.emplace_back(level(),levelCount);}
};
using UpgradeSoundCalls=std::vector<std::array<unsigned,2>>;
UpgradeSoundCalls upgradeActualSounds,upgradeNativeSounds;
struct UpgradeAudio final:SoundManager {
    bool native=false;
    UpgradeAudio(FileIndex& files,Settings& settings,SoundApi& api):SoundManager(files,settings,api,0) {}
    cChannel* playSound(cSound* sound,bool loop,bool start) override {
        require(sound && sound->resourceName().view()=="sounds/building_upgrade.wav","Upgrade sound resource differs");
        (native ? upgradeNativeSounds:upgradeActualSounds).push_back({unsigned(loop),unsigned(start)});
        return SoundManager::playSound(sound,loop,start);
    }
};
UpgradeAudio* upgradeAudio=nullptr;
cChannel* __fastcall upgradePlaySound(void*,void*,cSound* sound,bool loop,bool start) {
    upgradeAudio->native=true;auto* result=upgradeAudio->playSound(sound,loop,start);upgradeAudio->native=false;return result;
}
}
