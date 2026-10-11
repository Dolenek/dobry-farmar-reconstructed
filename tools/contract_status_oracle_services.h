#include "ContractStatus.h"
namespace {
using ContractCalls=std::vector<std::array<unsigned,5>>;
ContractCalls contractSourceCalls,contractNativeCalls;
ProfileProperties* contractNativeProfile=nullptr;void* contractNativeStatus=nullptr;
bool contractTutorial=false,contractTutorialStep=false,contractCanShow=false;
std::array<unsigned char,3072> contractTutorialObject{};
void* __fastcall contractProperty(void* storage,void*,const String* key) {
    const std::string name(key->view());
    if(storage==image+0x25ff20) return &contractNativeProfile->ints[name];
    require(storage==image+0x25ff44,"Contract integer property store differs");return &contractNativeProfile->uints[name];
}
float* __fastcall contractFloatProperty(void*,void*,const String* key) {return &contractNativeProfile->floats[std::string(key->view())];}
bool* __fastcall contractBoolProperty(void*,void*,const String* key) {return &contractNativeProfile->booleans[std::string(key->view())];}
void __fastcall contractClearStream(void*,void*) {listKey.str(L"");listKey.clear();}
void __fastcall contractSetStream(void*,void*,const wchar_t*,unsigned,void*) {}
void* __fastcall contractWriteSigned(void* self,void*,int value) {listKey<<value;return self;}
bool __fastcall contractNativeCanShow(void*,void*,unsigned step) {require(step==0x12,"Contract tutorial requested other step");return contractCanShow;}
bool __fastcall contractNativeStep(void*,void*,unsigned step) {require(step==0x12,"Contract tutorial click step differs");return contractTutorialStep;}
void __fastcall contractNativeShow(void*,void*,unsigned step,void* anchor) {
    require(step==0x12 && anchor==field<void*>(contractNativeStatus,0x34),"Contract tutorial anchor differs");contractNativeCalls.push_back({1,step,0,0,0});
}
void __cdecl contractNativeMoney(int value) {contractNativeCalls.push_back({2,unsigned(value),0,0,0});contractNativeProfile->ints["Money"]=value<0 ? 0:value;}
void __fastcall contractNativeStock(void*,void*,unsigned product,unsigned amount,bool a,bool b) {contractNativeCalls.push_back({3,product,amount,unsigned(a),unsigned(b)});}
void __fastcall contractNativeRemove(void*,void*,void* status) {require(status==contractNativeStatus,"Contract remove pointer differs");contractNativeCalls.push_back({4,0,0,0,0});}
void __fastcall contractNativeNotify(void*,void*,const WString* note,bool flag) {
    const auto& eventNote=field<WString>(field<void*>(contractNativeStatus,0x6c),0x68);
    require(note->view()==eventNote.view() || note->view()==field<WString>(field<void*>(contractNativeStatus,0x6c),0x84).view(),"Contract notification note differs");
    contractNativeCalls.push_back({5,unsigned(flag),unsigned(note->view()==eventNote.view()),unsigned(note->size()),0});
}
void __fastcall contractNativeReady(void*,void*,void* status) {require(status==contractNativeStatus,"Contract ready pointer differs");contractNativeCalls.push_back({6,0,0,0,0});}
void __fastcall contractNativeRequirements(void*,void*,void* status) {require(status==contractNativeStatus,"Contract requirements pointer differs");contractNativeCalls.push_back({7,0,0,0,0});}
void __fastcall contractNativeResult(void*,void*,void* status) {require(status==contractNativeStatus,"Contract result pointer differs");contractNativeCalls.push_back({8,0,0,0,0});}
struct ContractTrace final:ContractStatusWorld {
    ProfileProperties& profile;std::array<View*,3> transitions;
    ContractTrace(ProfileProperties& value,std::array<View*,3> views):profile(value),transitions(views) {}
    bool tutorialActive() const override {return contractTutorial;}
    bool tutorialStepActive(unsigned step) const override {require(step==0x12,"Source tutorial click step differs");return contractTutorialStep;}
    bool tutorialCanShow(unsigned step) const override {require(step==0x12,"Source tutorial show step differs");return contractCanShow;}
    void showTutorial(unsigned step,View&) override {contractSourceCalls.push_back({1,step,0,0,0});}
    bool worldTransitioning() const override {return std::any_of(transitions.begin(),transitions.end(),[](auto* view){return view->state().transition!=0;});}
    float elapsed() const override {return currentElapsed();}
    void setMoney(int value) override {contractSourceCalls.push_back({2,unsigned(value),0,0,0});profile.ints["Money"]=value<0 ? 0:value;}
    void changeStock(unsigned product,unsigned amount,bool a,bool b) override {contractSourceCalls.push_back({3,product,amount,unsigned(a),unsigned(b)});}
    void removeStatus(ContractStatus&) override {contractSourceCalls.push_back({4,0,0,0,0});}
    void notify(const WString& note,bool flag) override {contractSourceCalls.push_back({5,unsigned(flag),unsigned(note.view()==std::get<ContractEvent>(source->event->data).winNote.view()),unsigned(note.size()),0});}
    void openReadyDialog(ContractStatus&) override {contractSourceCalls.push_back({6,0,0,0,0});}
    void openRequirementsDialog(ContractStatus&) override {contractSourceCalls.push_back({7,0,0,0,0});}
    void openResultDialog(ContractStatus&) override {contractSourceCalls.push_back({8,0,0,0,0});}
    ContractStatus* source=nullptr;
};
struct ContractAudio final:SoundManager {
    using SoundManager::SoundManager;
    void clickSound(cSound* sound) override {actualSounds.emplace_back(std::string(sound->resourceName().view()),1);SoundManager::clickSound(sound);}
};
}
