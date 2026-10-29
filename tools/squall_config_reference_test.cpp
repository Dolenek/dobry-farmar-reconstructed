#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "Squall.h"
#include <filesystem>
#include <float.h>
#include <array>
#include <iostream>
#include <stdexcept>
#include <cstdint>
#include <cstring>
namespace {
void need(bool ok,const char* text){if(!ok)throw std::runtime_error(text);}
struct Api {
#define SQUALL_ENTRY(name) decltype(&name) name{};
#include "squall_exports.inc"
#undef SQUALL_ENTRY
    HMODULE module{};
    explicit Api(const wchar_t* path){module=LoadLibraryW(std::filesystem::absolute(path).c_str());need(module!=nullptr,"Load DLL");
#define SQUALL_ENTRY(name) name=reinterpret_cast<decltype(name)>(GetProcAddress(module,#name));need(name!=nullptr,#name);
#include "squall_exports.inc"
#undef SQUALL_ENTRY
    }
    ~Api(){if(module){SQUALL_Free();FreeLibrary(module);}}
};
unsigned results{},snapshots{};
void same(int source,int native,const char* what,int input=0){++results;if(source!=native){std::cerr<<what<<" input="<<input<<" source="<<source<<" native="<<native<<'\n';throw std::runtime_error("Configuration differs");}}
void getters(Api& s,Api& n){
    same(s.SQUALL_GetBufferSize(),n.SQUALL_GetBufferSize(),"GetBufferSize");
    same(s.SQUALL_Get3DAlgorithm(),n.SQUALL_Get3DAlgorithm(),"Get3DAlgorithm");
    same(s.SQUALL_GetSpeakerMode(),n.SQUALL_GetSpeakerMode(),"GetSpeakerMode");
    same(s.SQUALL_GetDevice(),n.SQUALL_GetDevice(),"GetDevice");
}
void outputs(Api& s,Api& n){
    struct Guard {std::uint32_t left=0x139acfe8;int a=0x12345678,b=0x23456789;std::uint32_t right=0xfc98ab15;};
    for(unsigned mask=0;mask<4;++mask){
        Guard source,native;
        same(s.SQUALL_GetHardwareAcceleration(mask&1 ? &source.a:nullptr,mask&2 ? &source.b:nullptr),n.SQUALL_GetHardwareAcceleration(mask&1 ? &native.a:nullptr,mask&2 ? &native.b:nullptr),"GetHW pointer mask",mask);
        need(std::memcmp(&source,&native,sizeof(source))==0,"HW output/guard differs");++snapshots;
    }
    Guard source,native;
    same(s.SQUALL_GetHardwareAcceleration(&source.a,&source.a),n.SQUALL_GetHardwareAcceleration(&native.a,&native.a),"GetHW alias");need(!std::memcmp(&source,&native,sizeof(source)),"HW alias output differs");++snapshots;
    same(s.SQUALL_GetHardwareAcceleration(reinterpret_cast<int*>(1),nullptr),n.SQUALL_GetHardwareAcceleration(reinterpret_cast<int*>(1),nullptr),"GetHW inaccessible first");
    same(s.SQUALL_GetHardwareAcceleration(nullptr,reinterpret_cast<int*>(1)),n.SQUALL_GetHardwareAcceleration(nullptr,reinterpret_cast<int*>(1)),"GetHW inaccessible second");
    auto* pages=static_cast<unsigned char*>(VirtualAlloc(nullptr,8192,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));need(pages!=nullptr,"Allocate guarded pages");DWORD old{};need(VirtualProtect(pages+4096,4096,PAGE_NOACCESS,&old)!=FALSE,"Guard second page");
    int* cross=reinterpret_cast<int*>(pages+4094);
    same(s.SQUALL_GetHardwareAcceleration(cross,nullptr),n.SQUALL_GetHardwareAcceleration(cross,nullptr),"GetHW cross-page");
    VirtualFree(pages,0,MEM_RELEASE);
}
void cases(Api& s,Api& n){
    getters(s,n);outputs(s,n);
    for(int value:{INT_MIN,-5001,-1,0,1,2,199,200,4999,5000,5001,65536,INT_MAX}){
        same(s.SQUALL_SetBufferSize(value),n.SQUALL_SetBufferSize(value),"SetBufferSize",value);getters(s,n);
    }
    for(int value:{INT_MIN,-1,0,1,2,3,4,5,65536,INT_MAX,0,0,3,3}){
        same(s.SQUALL_Set3DAlgorithm(value),n.SQUALL_Set3DAlgorithm(value),"Set3DAlgorithm",value);getters(s,n);
    }
    for(int value:{INT_MIN,-1,0,1,2,3,4,5,6,7,8,65536,INT_MAX}){
        same(s.SQUALL_SetSpeakerMode(value),n.SQUALL_SetSpeakerMode(value),"SetSpeakerMode",value);getters(s,n);
    }
    for(int a:{0,1,-1,2,INT_MIN,INT_MAX})for(int b:{0,1,-1,2,INT_MIN,INT_MAX}){
        same(s.SQUALL_SetHardwareAcceleration(a,b),n.SQUALL_SetHardwareAcceleration(a,b),"SetHW",a);outputs(s,n);getters(s,n);
    }
    const auto count=n.SQUALL_GetNumDevice();same(s.SQUALL_GetNumDevice(),count,"Device count");need(count>0,"No actual audio device");
    for(int value:{INT_MIN,-1,0,count-1,count,count+1,INT_MAX}){
        same(s.SQUALL_SetDevice(value),n.SQUALL_SetDevice(value),"SetDevice",value);getters(s,n);outputs(s,n);
    }
}
}
int wmain(int argc,wchar_t** argv){
    if(argc<3||argc>4)return 2;HWND window{};
    try{
        Api native(argv[1]),source(argv[2]);_control87(argc==4 ? _PC_24:_PC_53,_MCW_PC);
        cases(source,native);
        window=CreateWindowExW(0,L"STATIC",L"Squall actual configuration",0,0,0,64,64,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);need(window!=nullptr,"Create actual audio window");
        squall_parameters_t p{};p.Window=window;p.SampleRate=44100;p.BitPerSample=16;p.Channels=4;p.UseAlg=3;p.BufferSize=200;p.DistanceFactor=1;p.RolloffFactor=.001f;p.DopplerFactor=1;
        auto initialized=native.SQUALL_Init(&p);same(source.SQUALL_Init(&p),initialized,"Actual Init");need(initialized>=0,"Actual audio Init failed");
        cases(source,native);source.SQUALL_Free();native.SQUALL_Free();cases(source,native);
        need((_control87(0,0)&_MCW_PC)==(argc==4 ? _PC_24:_PC_53),"Precision changed");
        DestroyWindow(window);window=nullptr;
        std::cout<<"{\"result_comparisons\":"<<results<<",\"guarded_output_snapshots\":"<<snapshots<<",\"export_lookups\":194,\"actual_directsound\":true}\n";return 0;
    }catch(const std::exception& error){if(window)DestroyWindow(window);std::cerr<<error.what()<<'\n';return 1;}
}
