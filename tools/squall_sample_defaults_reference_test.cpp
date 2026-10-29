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
void require(bool value,const char* text){if(!value)throw std::runtime_error(text);}
struct Api {
#define SQUALL_ENTRY(name) decltype(&name) name{};
#include "squall_exports.inc"
#undef SQUALL_ENTRY
    HMODULE module{};
    explicit Api(const wchar_t* path){
        module=LoadLibraryW(std::filesystem::absolute(path).c_str());require(module!=nullptr,"LoadLibrary failed");
#define SQUALL_ENTRY(name) name=reinterpret_cast<decltype(name)>(GetProcAddress(module,#name));require(name!=nullptr,#name);
#include "squall_exports.inc"
#undef SQUALL_ENTRY
    }
    ~Api(){if(module){SQUALL_Sample_UnloadAll();SQUALL_Free();FreeLibrary(module);}}
};
unsigned cases=0,comparisons=0,fieldIndex=0,mismatches=0,pointerCases=0,loadDefaultCases=0;std::uint32_t inputBits{};bool diagnose=false;
void same(int a,int b,const char* label){++comparisons;if(a!=b){std::cerr<<label<<" source="<<a<<" native="<<b<<'\n';throw std::runtime_error("Sample default result differs");}}
void one(Api& source,Api& native,int ownSample,int originalSample,const std::array<std::uint32_t,7>& input){
    std::array<std::uint32_t,11> a,b;a.fill(0x5a5a5a5a);std::memcpy(a.data()+2,input.data(),28);b=a;const auto originalInput=a;
    same(source.SQUALL_Sample_SetDefault(ownSample,reinterpret_cast<squall_sample_default_t*>(a.data()+2)),native.SQUALL_Sample_SetDefault(originalSample,reinterpret_cast<squall_sample_default_t*>(b.data()+2)),"Sample_SetDefault");
    require(a==originalInput&&b==originalInput,"Setter input or guard changed");a.fill(0x5a5a5a5a);b=a;
    same(source.SQUALL_Sample_GetDefault(ownSample,reinterpret_cast<squall_sample_default_t*>(a.data()+2)),native.SQUALL_Sample_GetDefault(originalSample,reinterpret_cast<squall_sample_default_t*>(b.data()+2)),"Sample_GetDefault");
    if(a!=b){++mismatches;if(mismatches<=8)for(unsigned i=0;i<a.size();++i)if(a[i]!=b[i])std::cerr<<"Default word="<<i<<" input_field="<<fieldIndex<<" input_bits="<<std::hex<<inputBits<<" source="<<a[i]<<" native="<<b[i]<<std::dec<<'\n';if(!diagnose)throw std::runtime_error("Normalized sample default words differ");}
    ++cases;
}
void pointers(Api& source,Api& native,int actual,int expected){
    SYSTEM_INFO info{};GetSystemInfo(&info);const DWORD page=info.dwPageSize;
    auto* pages=static_cast<char*>(VirtualAlloc(nullptr,page*3,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));require(pages!=nullptr,"Allocate protected pointer pages");
    DWORD previous{};require(VirtualProtect(pages,page,PAGE_READONLY,&previous)!=FALSE,"Protect read-only page");require(VirtualProtect(pages+2*page,page,PAGE_NOACCESS,&previous)!=FALSE,"Protect guard page");
    for(unsigned id=0;id<5;++id)for(unsigned mode=0;mode<5;++mode){
        std::array<std::uint32_t,11> a,b;a.fill(0x5a5a5a5a);b=a;
        auto pointer=[&](auto& words)->squall_sample_default_t*{
            if(mode==0)return reinterpret_cast<squall_sample_default_t*>(words.data()+2);
            if(mode==1)return nullptr;
            if(mode==2)return reinterpret_cast<squall_sample_default_t*>(std::uintptr_t(1));
            if(mode==3)return reinterpret_cast<squall_sample_default_t*>(pages);
            return reinterpret_cast<squall_sample_default_t*>(pages+2*page-24);
        };
        const std::array<int,4> invalid{0,-1,0x7fffffff,0x10000};const int ownId=id ? invalid[id-1]:actual,nativeId=id ? invalid[id-1]:expected;
        same(source.SQUALL_Sample_SetDefault(ownId,pointer(a)),native.SQUALL_Sample_SetDefault(nativeId,pointer(b)),"Guarded sample default setter");require(a==b,"Setter guarded words differ");++pointerCases;
        same(source.SQUALL_Sample_GetDefault(ownId,pointer(a)),native.SQUALL_Sample_GetDefault(nativeId,pointer(b)),"Guarded sample default getter");require(a==b,"Getter guarded words differ");++pointerCases;
    }
    VirtualFree(pages,0,MEM_RELEASE);
}
void loadDefaults(Api& source,Api& native){
    for(unsigned field=5;field<7;++field)for(unsigned bits:{0U,0x80000000U,1U,0x80000001U,0x3f800000U,0x7f800000U,0xff800000U,0x7fc00123U,0x7f800123U,0xffc00123U,0xff800123U}){
        std::array<std::uint32_t,7> a{17,7,44100,100,50,0x3f800000,0x4e6e6b28};a[field]=bits;auto b=a;const auto initial=a;char file[]="sample.ogg";
        const int expected=native.SQUALL_Sample_LoadFile(file,1,reinterpret_cast<squall_sample_default_t*>(b.data()));
        const int actual=source.SQUALL_Sample_LoadFile(file,1,reinterpret_cast<squall_sample_default_t*>(a.data()));same(actual,expected,"LoadFile explicit default");require(actual>0,"Explicit-default sample failed");require(a==initial&&b==initial,"LoadFile default input changed");
        std::array<std::uint32_t,11> own,original;own.fill(0x5a5a5a5a);original=own;
        same(source.SQUALL_Sample_GetDefault(actual,reinterpret_cast<squall_sample_default_t*>(own.data()+2)),native.SQUALL_Sample_GetDefault(expected,reinterpret_cast<squall_sample_default_t*>(original.data()+2)),"Get explicit load default");require(own==original,"LoadFile normalized default or guards differ");
        source.SQUALL_Sample_Unload(actual);native.SQUALL_Sample_Unload(expected);++loadDefaultCases;
    }
}
}
int wmain(int argc,wchar_t** argv){
    if(argc<3||argc>4)return 2;
    try{
        Api native(argv[1]),source(argv[2]);_control87(argc==4 ? _PC_24:_PC_53,_MCW_PC);diagnose=GetEnvironmentVariableW(L"FARMAR_SAMPLE_DEFAULTS_DIAGNOSE",nullptr,0)!=0;
        char file[]="sample.ogg";const int expected=native.SQUALL_Sample_LoadFile(file,1,nullptr),actual=source.SQUALL_Sample_LoadFile(file,1,nullptr);same(actual,expected,"Actual pre-Init sample load");require(expected>0,"Actual sample file load failed");
        const std::array<std::uint32_t,7> initial{17,7,0,100,50,0x3f800000,0x4e6e6b28};
        for(fieldIndex=0;fieldIndex<5;++fieldIndex)for(auto word:{0U,1U,99U,100U,101U,100000U,100001U,0x7fffffffU,0x80000000U,0xffffffffU}){auto value=initial;inputBits=word;value[fieldIndex]=word;one(source,native,actual,expected,value);}
        std::vector<std::uint32_t> values{0,0x80000000,1,0x80000001,0x007fffff,0x00800000,0x80800000,0x3dcccccd,0x3f800000,0x41200000,0x411fffff,0x41200001,0x7f7fffff,0xff7fffff,0x7f800000,0xff800000};
        for(auto sign:{0U,0x80000000U})for(auto quiet:{0U,0x00400000U})for(unsigned payload=1;payload<=256;++payload)values.push_back(sign|0x7f800000U|quiet|payload);
        for(fieldIndex=5;fieldIndex<7;++fieldIndex)for(auto word:values){auto value=initial;inputBits=word;value[fieldIndex]=word;one(source,native,actual,expected,value);}
        if(!diagnose){pointers(source,native,actual,expected);loadDefaults(source,native);}
        require((_control87(0,0)&_MCW_PC)==(argc==4 ? _PC_24:_PC_53),"x87 precision changed");
        std::cout<<"{\"cases\":"<<cases<<",\"result_comparisons\":"<<comparisons<<",\"pointer_cases\":"<<pointerCases<<",\"explicit_load_default_cases\":"<<loadDefaultCases<<",\"normalization_mismatches\":"<<mismatches<<",\"initialized_directsound\":false,\"export_lookups\":194}\n";return mismatches ? 1:0;
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
