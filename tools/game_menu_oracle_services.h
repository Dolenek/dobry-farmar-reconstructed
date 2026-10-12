#include "GameMenu.h"
#include "TutorialManager.h"
namespace {
using HudCalls=std::vector<std::array<unsigned,3>>;
HudCalls hudSourceCalls,hudNativeCalls;
struct HudInputs {
    std::array<int,5> wealth{};int level=0;unsigned free=1;
    unsigned capacity=100;
    std::array<unsigned,19> products{};std::array<bool,19> available{};
    bool character=false,tip=false,active=false,blocked=false;
    unsigned long long can=0,steps=0;
    std::array<bool,5> tools{};
};
HudInputs hudInputs;bool hudUseActualWarehouse=false;
std::map<int,void*> hudNativeEvents;
void* __fastcall hudSavedEvent(void*,void*,int index) {hudNativeCalls.push_back({25,unsigned(index),0});return hudNativeEvents.at(index);}
void* __cdecl hudLiteral(void* self,const char* text) {listKey<<text;return self;}
int __cdecl hudMoney() {hudNativeCalls.push_back({10,0,0});return contractNativeProfile->ints["Money"];}
template<unsigned N> int __fastcall hudValue(void*,void*) {hudNativeCalls.push_back({10,N,0});return hudInputs.wealth[N];}
int __cdecl hudLevel() {hudNativeCalls.push_back({11,0,0});return hudInputs.level;}
unsigned __fastcall hudFree(void*,void*) {hudNativeCalls.push_back({12,0,0});return hudInputs.free;}
unsigned __fastcall hudProducts(void*,void*,unsigned n) {hudNativeCalls.push_back({13,n,0});return hudInputs.products.at(n);}
bool __fastcall hudAvailable(void*,void*,unsigned n) {hudNativeCalls.push_back({14,n,0});return hudInputs.available.at(n);}
bool __fastcall hudCan(void*,void*,unsigned n) {hudNativeCalls.push_back({15,n,0});return bool(hudInputs.can&(1ULL<<n));}
bool __fastcall hudStep(void*,void*,unsigned n) {hudNativeCalls.push_back({16,n,0});return bool(hudInputs.steps&(1ULL<<n));}
void __fastcall hudShow(void*,void*,unsigned n,void* anchor) {
    auto* menu=field<void*>(image,0x25f740);unsigned slot=0;for(unsigned s:{0x3cU,0x40U,0x44U,0x48U,0x68U}) if(field<void*>(menu,s)==anchor) slot=s;
    require(slot,"HUD tutorial anchor is not actual menu control");hudNativeCalls.push_back({17,n,slot});
}
void __fastcall hudComplete(void*,void*,unsigned n) {hudNativeCalls.push_back({18,n,0});}
void __cdecl hudMessages() {hudNativeCalls.push_back({19,0,0});}
void __fastcall hudGameplay(void*,void*) {hudNativeCalls.push_back({20,0,0});}
void __fastcall hudTipShow(void* self,void*) {field<unsigned char>(self,0x370)=1;hudNativeCalls.push_back({21,1,0});}
void __fastcall hudTipHide(void* self,void*) {field<unsigned char>(self,0x370)=0;hudNativeCalls.push_back({21,0,0});}
void __fastcall hudTipReset(void* self,void*) {field<unsigned char>(self,0x370)=0;hudNativeCalls.push_back({22,0,0});}
template<unsigned N> void __fastcall hudStop(void*,void*) {hudNativeCalls.push_back({23,N,0});}
void __fastcall hudStopSix(void*,void*,unsigned n) {hudNativeCalls.push_back({23,n,0});}
void __fastcall hudResume(void*,void*,unsigned n) {hudNativeCalls.push_back({24,n,0});}
struct HudWorld final:GameMenuWorld,ContractTrace {
    HudWorld(ProfileProperties& profile,std::array<StateView*,3> scenes,std::array<View*,3> overlays)
        :ContractTrace(profile,{scenes[1],scenes[0],scenes[2]}),scenes(scenes),overlays(overlays) {}
    std::array<StateView*,3> scenes;std::array<View*,3> overlays;GameMenu* menu=nullptr;TutorialManager* tutorial=nullptr;
    std::map<int,GameplayEvent*> events;
    StateView& farm() override {return *scenes[0];}StateView& wareDetails() override {return *scenes[1];}StateView& manufacture() override {return *scenes[2];}
    View& sceneOverlay() override {return *overlays[0];}View& selectionOverlay() override {return *overlays[1];}View& warehouseTip() override {return *overlays[2];}
    bool warehouseTipOpen() const override {return hudInputs.tip;}
    void showWarehouseTip() override {hudInputs.tip=true;hudSourceCalls.push_back({21,1,0});}
    void hideWarehouseTip() override {hudInputs.tip=false;hudSourceCalls.push_back({21,0,0});}
    void resetWarehouseTip() override {hudInputs.tip=false;hudSourceCalls.push_back({22,0,0});}
    bool inputBlocked() const override {return hudInputs.blocked;}
    int money() const override {hudSourceCalls.push_back({10,0,0});return profile.ints["Money"];}
    int warehouseValue() const override {if(hudUseActualWarehouse) return int(Warehouse::instance()->investment());hudSourceCalls.push_back({10,1,0});return hudInputs.wealth[1];}
    int wareValue() const override {hudSourceCalls.push_back({10,2,0});return hudInputs.wealth[2];}
    int characterValue() const override {hudSourceCalls.push_back({10,3,0});return hudInputs.wealth[3];}
    int farmValue() const override {hudSourceCalls.push_back({10,4,0});return hudInputs.wealth[4];}
    int wareLevel() const override {if(hudUseActualWarehouse) return Warehouse::instance()->level();hudSourceCalls.push_back({11,0,0});return hudInputs.level;}
    unsigned warehouseFree() const override {if(hudUseActualWarehouse) return Warehouse::instance()->freeSpace();hudSourceCalls.push_back({12,0,0});return hudInputs.free;}
    unsigned warehouseCapacity(int level) const override {return hudUseActualWarehouse ? Warehouse::instance()->levels.at(unsigned(level)).capacity:hudInputs.capacity;}
    unsigned manufactureProductCount(unsigned n) override {hudSourceCalls.push_back({13,n,0});return hudInputs.products.at(n);}
    bool manufactureProductAvailable(unsigned n) override {hudSourceCalls.push_back({14,n,0});return hudInputs.available.at(n);}
    bool mainCharacterPresent() const override {return hudInputs.character;}
    bool toolRunning(unsigned n) const override {return hudInputs.tools.at(n);}
    void resumeTool(unsigned n) override {hudSourceCalls.push_back({24,n,0});}
    void stopTool(unsigned n) override {hudSourceCalls.push_back({23,n,0});}
    GameplayEvent& savedContract(int index) override {hudSourceCalls.push_back({25,unsigned(index),0});return *events.at(index);}
    void stepGameplay() override {hudSourceCalls.push_back({20,0,0});}
    void completeTutorial(unsigned n) override {if(tutorial) {tutorial->complete(n);return;}hudSourceCalls.push_back({18,n,0});}
    void openMessages() override {hudSourceCalls.push_back({19,0,0});}
    bool tutorialActive() const override {if(tutorial) return tutorial->active();return hudInputs.active;}
    bool tutorialStepActive(unsigned n) const override {if(tutorial) return tutorial->activeStep(int(n));hudSourceCalls.push_back({16,n,0});return bool(hudInputs.steps&(1ULL<<n));}
    bool tutorialCanShow(unsigned n) const override {if(tutorial) return tutorial->canShow(n);hudSourceCalls.push_back({15,n,0});return bool(hudInputs.can&(1ULL<<n));}
    void showTutorial(unsigned n,View& anchor) override {
        if(tutorial) {tutorial->showAction(n,dynamic_cast<Control*>(&anchor));return;}
        unsigned slot=0;for(auto pair:std::initializer_list<std::pair<View*,unsigned>>{{&menu->mainButton(),0x3c},{&menu->marketButton(),0x40},{&menu->farmButton(),0x44},{&menu->wareButton(),0x48},{&menu->journalButton(),0x68}}) if(pair.first==&anchor) slot=pair.second;
        require(slot,"Source HUD tutorial anchor differs");hudSourceCalls.push_back({17,n,slot});
    }
    bool worldTransitioning() const override {return scenes[0]->movement || scenes[1]->movement || scenes[2]->movement;}
    void removeStatus(ContractStatus& value) override {menu->removeContract(value);}
    void notify(const WString& value,bool flag) override {menu->notify(value,flag);}
    void openReadyDialog(ContractStatus& status) override {ContractPassConfirm::instance()->start(status);}
};
}
