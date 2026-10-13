#include "Gameplay.h"
namespace {
std::array<unsigned char,64> gameplayThreadData{};
void* __cdecl gameplayGetThreadData() {return gameplayThreadData.data();}
ProfileProperties* gameplayNativeProfile=nullptr;
float* __fastcall gameplayFloatProperty(void*,void*,const String* name) {
    require(name->view()=="GameTime","Gameplay float property key differs");return &gameplayNativeProfile->floats[std::string(name->view())];
}
int* __fastcall gameplayIntProperty(void*,void*,const String* name) {return &gameplayNativeProfile->ints[std::string(name->view())];}
const XmlNode* gameplayRuntimeDefinition=nullptr;unsigned gameplayXmlReleased=0;
XmlAdapter* __fastcall gameplayRuntimeXML(void*,void*,const String* name) {
    require(name->view()=="Gameplay/Gameplay.xml" && gameplayRuntimeDefinition,"Gameplay constructor XML differs");return wrap(gameplayRuntimeDefinition);
}
void __fastcall gameplayReleaseXML(void*,void*) {++gameplayXmlReleased;}
using GameplayCalls=std::vector<std::array<unsigned,2>>;
GameplayCalls gameplayNativeCalls;
bool gameplayMutateTime=false;
void __fastcall gameplayDispatch(void* event,void*) {
    gameplayNativeCalls.push_back({field<unsigned>(event,4),field<unsigned>(event,0x34)});
    if(gameplayMutateTime) gameplayNativeProfile->floats["GameTime"]=-125.5f;
}
struct GameplayTrace final:GameplayEventExecutor {
    ProfileProperties& profile;GameplayCalls calls;int contractState=0;
    explicit GameplayTrace(ProfileProperties& value):profile(value) {}
    bool contractAvailable() const override {return contractState!=3;}
    void execute(GameplayEvent& event) override {calls.push_back({unsigned(event.type),event.index});if(gameplayMutateTime) profile.floats["GameTime"]=-125.5f;}
};
}
