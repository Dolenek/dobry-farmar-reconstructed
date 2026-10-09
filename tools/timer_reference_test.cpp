// Native cTimer is a test oracle only. Original entry point is not executed;
// clock/Sleep/WinMM and settings calls are explicit, deterministic boundaries.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "GameTimer.h"
#include "Settings.h"
#include "FileIndex.h"
#include "XmlDocument.h"
#include <array>
#include <cstring>
#include <float.h>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>
using namespace farmar::recovered;
namespace {
unsigned char* image=nullptr;
unsigned scenario=0,frame=0;
template<class T> T& field(void* object,unsigned offset) { return *reinterpret_cast<T*>(static_cast<unsigned char*>(object)+offset); }
template<class T> T function(unsigned rva) { return reinterpret_cast<T>(image+rva); }
void require(bool result,const char* message) { if(!result) throw std::runtime_error(message); }
void write(unsigned rva,const void* value,unsigned count) {
    DWORD old,unused;require(VirtualProtect(image+rva,count,PAGE_EXECUTE_READWRITE,&old)!=0,"Timer oracle protection failed");
    std::memcpy(image+rva,value,count);VirtualProtect(image+rva,count,old,&unused);
}
void pointer(unsigned rva,void* value) { write(rva,&value,4); }
void redirect(unsigned rva,void* value) {
    unsigned char jump[5]={0xe9};const auto delta=reinterpret_cast<unsigned>(value)-reinterpret_cast<unsigned>(image+rva+5);
    std::memcpy(jump+1,&delta,4);write(rva,jump,5);
}
using Trace=std::vector<std::pair<unsigned,unsigned>>;
Trace actualTrace,expectedTrace;Trace* trace=&actualTrace;
std::array<DWORD,2> ticks{};unsigned tickIndex=0;
DWORD WINAPI clockValue() { const auto value=ticks[tickIndex<2 ? tickIndex++:1];trace->emplace_back(0,value);return value; }
void WINAPI sleepValue(DWORD milliseconds) { trace->emplace_back(1,milliseconds); }
MMRESULT WINAPI beginValue(UINT period) { trace->emplace_back(2,period);return 5; }
MMRESULT WINAPI endValue(UINT period) { trace->emplace_back(3,period);return 5; }
String* __fastcall assign(String* self,void*,const char* bytes,unsigned count) { return &self->assign(bytes,count); }
struct SettingsAdapter { void** table;Settings* settings; };
bool __fastcall getFloat(SettingsAdapter* self,void*,const String* section,const String* property,float* output) { return self->settings->get(*section,*property,*output); }
void __fastcall setFloat(SettingsAdapter* self,void*,const String* section,const String* property,const float* value) { self->settings->set(*section,*property,*value); }
void sameFloat(float actual,float expected,const char* label) {
    if(std::memcmp(&actual,&expected,4)) { std::cerr<<label<<" actual "<<std::hexfloat<<actual<<" native "<<expected<<'\n';throw std::runtime_error("Native timer float differs"); }
}
void compare(const GameTimer& timer,void* native) {
    const auto& state=timer.state();
    const std::pair<float,unsigned> fields[]={{state.total,4},{state.previousTotal,0x14},{state.frame,0x18},
        {state.minFps,0x20},{state.maxFps,0x24},{state.maxFrame,0x28},{state.waitThreshold,0x2c},
        {state.multiplier,0x30},{state.fps,0x34},{state.average,0x38}};
    for(const auto& item:fields) sameFloat(item.first,field<float>(native,item.second),"timer field");
    require(state.lastMilliseconds==field<unsigned>(native,0x1c) && state.averageSamples==field<unsigned>(native,0x3c),"Timer integer state differs");
    sameFloat(timer.elapsed(),function<float(__thiscall*)(void*)>(0x136480)(native),"elapsed getter");
    sameFloat(timer.unscaledElapsed(),function<float(__thiscall*)(void*)>(0x1364a0)(native),"unscaled elapsed getter");
    sameFloat(timerAccumulation().seconds,field<float>(image,0x260608),"shared accumulation");
    require(timerAccumulation().samples==field<unsigned>(image,0x26060c),"Shared timer count differs");
    require(actualTrace==expectedTrace,"Timer clock/Sleep/WinMM call order differs");
}
bool sameSettings(const Settings& actual,const Settings& expected) {
    if(actual.sections().size()!=expected.sections().size()) return false;
    for(const auto& section:actual.sections()) {
        const auto match=expected.sections().find(section.first);
        if(match==expected.sections().end() || section.second.size()!=match->second.size()) return false;
        for(const auto& property:section.second) {
            const auto item=match->second.find(property.first);
            if(item==match->second.end() || property.second.view()!=item->second.view()) return false;
        }
    }
    return true;
}
}
int wmain(int argc,wchar_t** argv) {
    if(argc!=2 && argc!=3) return 2;
    const auto original=LoadLibraryExW(argv[1],nullptr,DONT_RESOLVE_DLL_REFERENCES);if(!original) return 2;
    try {
        if(argc==3) _control87(_PC_24,_MCW_PC);
        // Force a distinct address on every run: a preferred-base mapping could
        // otherwise hide missing absolute operands or callback pointer fixups.
        const auto loaded=reinterpret_cast<unsigned char*>(original);
        const auto header=reinterpret_cast<const IMAGE_NT_HEADERS*>(loaded+reinterpret_cast<const IMAGE_DOS_HEADER*>(loaded)->e_lfanew);
        image=static_cast<unsigned char*>(VirtualAlloc(nullptr,header->OptionalHeader.SizeOfImage,MEM_RESERVE|MEM_COMMIT,PAGE_EXECUTE_READWRITE));
        require(image && image!=loaded,"Timer oracle private relocation failed");
        std::memcpy(image,loaded,header->OptionalHeader.SizeOfImage);
        struct Fixup { unsigned rva,address; };const Fixup fixups[]={
#include "timer_oracle_fixups.inc"
        };
        for(const auto& item:fixups) {
            require(field<unsigned>(image,item.rva)==item.address,"Timer oracle operand differs");
            const auto value=reinterpret_cast<unsigned>(image)+item.address-0x400000;write(item.rva,&value,4);
        }
        redirect(0x19c0,reinterpret_cast<void*>(&assign));
        pointer(0x1ee2a8,reinterpret_cast<void*>(&clockValue));pointer(0x1ee0e4,reinterpret_cast<void*>(&sleepValue));
        pointer(0x1ee2ac,reinterpret_cast<void*>(&beginValue));pointer(0x1ee2a4,reinterpret_cast<void*>(&endValue));
        FlushInstructionCache(GetCurrentProcess(),image,0x200000);
        FileIndex files(String("."),String("pack"));
        const auto empty=[](const String&) { return std::make_shared<XmlNode>(); };
        std::array<void*,16> table{};table[5]=reinterpret_cast<void*>(&getFloat);table[15]=reinterpret_cast<void*>(&setFloat);
        SettingsAdapter adapter{table.data(),nullptr};pointer(0x25f728,&adapter);
        TimerServices services{&clockValue,&sleepValue,&beginValue,&endValue};
        std::mt19937 random(0x54494d45);unsigned comparisons=0,sleeps=0;
        for(scenario=0;scenario<100;++scenario) {
            Settings settings(files,empty),nativeSettings(files,empty);adapter.settings=&nativeSettings;
            const float minimums[]={15,0.25f,20.5f,1000,0,-3};const float maximums[]={200,60.5f,0.1f,0,-1};
            if(scenario%4!=0) { const float value=minimums[scenario%6];settings.set(String("Time"),String("MinFps"),value);nativeSettings.set(String("Time"),String("MinFps"),value); }
            if(scenario%5!=0) { const float value=maximums[scenario%5];settings.set(String("Time"),String("MaxFps"),value);nativeSettings.set(String("Time"),String("MaxFps"),value); }
            ticks={random(),0};tickIndex=0;trace=&actualTrace;actualTrace.clear();expectedTrace.clear();
            auto timer=std::make_unique<GameTimer>(settings,services);
            std::array<unsigned char,64> native{};tickIndex=0;trace=&expectedTrace;
            function<void*(__thiscall*)(void*)>(0x136ae0)(native.data());compare(*timer,native.data());++comparisons;
            require(sameSettings(settings,nativeSettings),"Timer constructor settings side effects differ");
            for(frame=0;frame<600;++frame) {
                if(frame%23==0) { const float multipliers[]={0,0.5f,1,3,-2};const float value=multipliers[(frame/23)%5];timer->setMultiplier(value);field<float>(native.data(),0x30)=value; }
                if(frame%79==0) { const float value=minimums[(frame/79+scenario)%6];timer->setMinFps(value);function<void(__thiscall*)(void*,const float*)>(0x136500)(native.data(),&value); }
                const auto last=timer->state().lastMilliseconds;
                const unsigned deltas[]={0,1,4,17,200,0x7fffffff,0x80000000,0xfffffff0};const unsigned delta=frame%17==0 ? deltas[(frame/17)%8]:random()%50;
                ticks={last+delta,last+delta+(frame%2 ? 0:3)};
                actualTrace.clear();expectedTrace.clear();trace=&actualTrace;tickIndex=0;timer->process();
                trace=&expectedTrace;tickIndex=0;function<void(__thiscall*)(void*)>(0x136720)(native.data());compare(*timer,native.data());++comparisons;
                for(const auto& call:actualTrace) if(call.first==1) ++sleeps;
            }
            actualTrace.clear();expectedTrace.clear();trace=&actualTrace;timer.reset();trace=&expectedTrace;
            function<void(__thiscall*)(void*)>(0x136430)(native.data());require(actualTrace==expectedTrace,"Timer destructor WinMM calls differ");
            require(!GameTimer::instance() && !field<void*>(image,0x25ff50),"Timer singleton cleanup differs");
        }
        std::cout<<"{\"constructors\":100,\"frames\":60000,\"state_comparisons\":"<<comparisons<<",\"sleep_calls\":"<<sleeps<<",\"destructors\":100}\n";
        VirtualFree(image,0,MEM_RELEASE);FreeLibrary(original);
    } catch(const std::exception& error) { std::cerr<<error.what()<<" scenario "<<scenario<<" frame "<<frame<<'\n';FreeLibrary(original);return 1; }
      catch(const char* error) { std::cerr<<error<<'\n';FreeLibrary(original);return 1; }
    return 0;
}
