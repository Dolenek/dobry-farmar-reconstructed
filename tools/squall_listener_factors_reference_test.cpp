// Public float-factor predicates, before/after actual DirectSound initialization.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "Squall.h"
#include <array>
#include <vector>
#include <filesystem>
#include <float.h>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
namespace {
struct Api {
#define SQUALL_ENTRY(name) decltype(&name) name{};
#include "squall_exports.inc"
#undef SQUALL_ENTRY
    HMODULE module{};
    explicit Api(const wchar_t* path){
        module=LoadLibraryW(std::filesystem::absolute(path).c_str());if(!module)throw std::runtime_error("LoadLibrary failed");
#define SQUALL_ENTRY(name) name=reinterpret_cast<decltype(name)>(GetProcAddress(module,#name));if(!name)throw std::runtime_error(#name);
#include "squall_exports.inc"
#undef SQUALL_ENTRY
    }
    ~Api(){if(module){SQUALL_Free();FreeLibrary(module);}}
};
using Setter=int(__cdecl*)(float);using Getter=int(__cdecl*)(float*);
unsigned comparisons=0,inputs=0,nanInputs=0,mismatches=0,initializedGetters=0;bool diagnose=false;std::uint32_t inputBits=0;
void require(bool good,const char* text){if(!good)throw std::runtime_error(text);}
void same(int a,int b,const char* name){++comparisons;if(a!=b){++mismatches;if(mismatches<=8)std::cerr<<name<<" float bits="<<std::hex<<inputBits<<std::dec<<" source="<<a<<" native="<<b<<'\n';if(!diagnose)throw std::runtime_error("Public listener factor differs");}}
std::vector<std::uint32_t> values(){
    std::vector<std::uint32_t> result{0,0x80000000,1,0x80000001,0x007fffff,0x00800000,0x80800000,0x3dcccccd,0x3f800000,0x41200000,0x411fffff,0x41200001,0x7f7fffff,0xff7fffff,0x7f800000,0xff800000};
    for(std::uint32_t sign:{0U,0x80000000U})for(std::uint32_t quiet:{0U,0x00400000U})for(std::uint32_t payload=1;payload<=256;++payload)result.push_back(sign|0x7f800000U|quiet|payload);
    return result;
}
void compare(Setter source,Setter native,Getter sourceGet,Getter nativeGet,bool initialized){
    for(auto bits:values()){
        inputBits=bits;float value;std::memcpy(&value,&bits,4);++inputs;
        const bool nan=(bits&0x7f800000U)==0x7f800000U && (bits&0x007fffffU)!=0;if(nan)++nanInputs;
        // Call independently; values remain exact uint32-backed binary32.
        const int expected=native(value),actual=source(value);same(actual,expected,"Listener factor setter");
        if(initialized){
            std::array<std::uint32_t,4> a,b;a.fill(0x5a5a5a5a);b=a;
            const int expectedGet=nativeGet(reinterpret_cast<float*>(b.data()+1));
            same(sourceGet(reinterpret_cast<float*>(a.data()+1)),expectedGet,"Listener factor getter");require(expectedGet>=0,"Actual initialized listener getter failed");require(a==b,"Cached listener factor or output guard differs");++initializedGetters;
        }
        if(nan)require(expected==-4,"Original did not reject NaN with invalid-parameter error");
    }
}
}
int wmain(int argc,wchar_t** argv){
    if(argc<3 || argc>4)return 2;HWND window=nullptr;
    try{
        Api native(argv[1]),source(argv[2]);_control87(argc==4 ? _PC_24:_PC_53,_MCW_PC);diagnose=GetEnvironmentVariableW(L"FARMAR_LISTENER_DIAGNOSE_PREINIT",nullptr,0)!=0;
        const std::array<Setter,3> ownSet{source.SQUALL_Listener_SetDistanceFactor,source.SQUALL_Listener_SetRolloffFactor,source.SQUALL_Listener_SetDopplerFactor};
        const std::array<Setter,3> originalSet{native.SQUALL_Listener_SetDistanceFactor,native.SQUALL_Listener_SetRolloffFactor,native.SQUALL_Listener_SetDopplerFactor};
        const std::array<Getter,3> ownGet{source.SQUALL_Listener_GetDistanceFactor,source.SQUALL_Listener_GetRolloffFactor,source.SQUALL_Listener_GetDopplerFactor};
        const std::array<Getter,3> originalGet{native.SQUALL_Listener_GetDistanceFactor,native.SQUALL_Listener_GetRolloffFactor,native.SQUALL_Listener_GetDopplerFactor};
        for(unsigned i=0;i<3;++i)compare(ownSet[i],originalSet[i],ownGet[i],originalGet[i],false);
        if(diagnose){std::cout<<"{\"preinit_inputs\":"<<inputs<<",\"nan_inputs\":"<<nanInputs<<",\"mismatches\":"<<mismatches<<",\"setter_count\":3}\n";return mismatches ? 1:0;}
        window=CreateWindowExW(0,L"STATIC",L"Squall listener factor comparison",0,0,0,64,64,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);require(window!=nullptr,"Actual hidden window required");
        squall_parameters_t parameters{};parameters.Window=window;parameters.SampleRate=44100;parameters.BitPerSample=16;parameters.Channels=16;parameters.UseAlg=3;parameters.BufferSize=200;parameters.DistanceFactor=1;parameters.RolloffFactor=.001f;parameters.DopplerFactor=1;
        const int initialized=native.SQUALL_Init(&parameters);same(source.SQUALL_Init(&parameters),initialized,"Actual DirectSound Init");require(initialized>=0,"Actual DirectSound initialization failed");
        for(unsigned i=0;i<3;++i)compare(ownSet[i],originalSet[i],ownGet[i],originalGet[i],true);
        source.SQUALL_Free();native.SQUALL_Free();for(unsigned i=0;i<3;++i)compare(ownSet[i],originalSet[i],ownGet[i],originalGet[i],false);
        require((_control87(0,0)&_MCW_PC)==(argc==4 ? _PC_24:_PC_53),"x87 precision changed");DestroyWindow(window);window=nullptr;
        std::cout<<"{\"result_comparisons\":"<<comparisons<<",\"factor_inputs\":"<<inputs<<",\"nan_inputs\":"<<nanInputs<<",\"initialized_getter_checks\":"<<initializedGetters<<",\"factor_setters\":3,\"actual_directsound\":true,\"export_lookups\":194}\n";return 0;
    }catch(const std::exception& error){if(window)DestroyWindow(window);std::cerr<<error.what()<<'\n';return 1;}
}
