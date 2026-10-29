// Actual DLL listener vectors, guarded pointer handling and deferred commit.
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
void require(bool ok,const char* text){if(!ok)throw std::runtime_error(text);}
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
    ~Api(){if(module){SQUALL_Free();FreeLibrary(module);}}
};
unsigned comparisons=0,snapshots=0,setterCalls=0,getterCases=0;
unsigned contextId=0,caseId=0;
void same(int source,int native,const char* label){
    ++comparisons;if(source!=native){std::cerr<<label<<" context="<<contextId<<" case="<<caseId<<" source="<<source<<" native="<<native<<'\n';throw std::runtime_error("Listener vector result differs");}
}
struct Pages {
    char* data{};DWORD size{};
    Pages(){SYSTEM_INFO info{};GetSystemInfo(&info);size=info.dwPageSize;data=static_cast<char*>(VirtualAlloc(nullptr,size*3,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));require(data!=nullptr,"VirtualAlloc failed");
        const std::array<float,3> sample{3.5f,-2,7};std::memcpy(data,sample.data(),12);DWORD previous{};
        require(VirtualProtect(data,size,PAGE_READONLY,&previous)!=FALSE,"Read-only page failed");require(VirtualProtect(data+size*2,size,PAGE_NOACCESS,&previous)!=FALSE,"Guard page failed");}
    ~Pages(){if(data)VirtualFree(data,0,MEM_RELEASE);}
    float* readOnly(){return reinterpret_cast<float*>(data);}
    float* cross(){return reinterpret_cast<float*>(data+size*2-8);}
};
using Words=std::array<std::uint32_t,20>;
std::array<float*,4> outputs(Words& words,unsigned mode,Pages& pages){
    std::array<float*,4> result{};for(unsigned i=0;i<4;++i)result[i]=reinterpret_cast<float*>(words.data()+i*4+1);
    if(mode<16){for(unsigned i=0;i<4;++i)if(!(mode&(1U<<i)))result[i]=nullptr;}
    else if(mode==16){for(auto& p:result)p=reinterpret_cast<float*>(std::uintptr_t(1));}
    else if(mode==17){for(auto& p:result)p=pages.readOnly();}
    else if(mode==18){for(auto& p:result)p=pages.cross();}
    else if(mode==19){for(auto& p:result)p=result[0];}
    else if(mode==20){for(unsigned i=0;i<4;++i)result[i]=reinterpret_cast<float*>(words.data()+1+i);}
    else throw std::runtime_error("Unknown pointer mode");
    return result;
}
void snapshot(Api& source,Api& native){
    Words a,b;a.fill(0x5a5a5a5a);b=a;
    const auto get=[](Api& api,Words& out){return api.SQUALL_Listener_GetParameters(reinterpret_cast<float*>(out.data()+1),reinterpret_cast<float*>(out.data()+5),reinterpret_cast<float*>(out.data()+9),reinterpret_cast<float*>(out.data()+13));};
    same(get(source,a),get(native,b),"GetParameters snapshot");require(a==b,"Cached vector bits or guards differ");++snapshots;
}
void getters(Api& source,Api& native,Pages& pages){
    for(unsigned mode=0;mode<=20;++mode){
        caseId=mode;Words a,b;a.fill(0x5a5a5a5a);b=a;auto own=outputs(a,mode,pages),original=outputs(b,mode,pages);
        same(source.SQUALL_Listener_GetParameters(own[0],own[1],own[2],own[3]),native.SQUALL_Listener_GetParameters(original[0],original[1],original[2],original[3]),"Guarded combined getter");require(a==b,"Combined getter bytes or guards differ");++getterCases;
        for(unsigned which=0;which<2;++which){
            a.fill(0x5a5a5a5a);b=a;own=outputs(a,mode,pages);original=outputs(b,mode,pages);
            same(which ? source.SQUALL_Listener_GetVelocity(own[0]):source.SQUALL_Listener_GetPosition(own[0]),which ? native.SQUALL_Listener_GetVelocity(original[0]):native.SQUALL_Listener_GetPosition(original[0]),"Guarded individual getter");require(a==b,"Individual getter bytes or guards differ");++getterCases;
        }
    }
}
void exercise(Api& source,Api& native,Pages& pages){
    getters(source,native,pages);snapshot(source,native);
    // Exact float words, including repeated/signed zero, nonfinite and subnormal values.
    const std::vector<std::array<std::uint32_t,3>> patterns{
        {0,0,0},{0x80000000,0,0x80000000},{0x3f800000,0x40000000,0xc0400000},
        {0x3f800000,0x40000000,0xc0400000},{1,0x80000001,0x007fffff},
        {0x7f800000,0xff800000,0},{0x7fc00001,0xffc00123,0x3f800000},
        {0x7f800001,0xff800123,0x3f800000},{0x7f7fffff,0xff7fffff,0},
        {0,0,0x3f800000},{0,0x3f800000,0}};
    for(unsigned n=0;n<patterns.size();++n){
        caseId=n;std::array<float,3> value{};std::memcpy(value.data(),patterns[n].data(),12);
        same(source.SQUALL_Listener_SetPosition(value.data()),native.SQUALL_Listener_SetPosition(value.data()),"SetPosition patterns");++setterCalls;snapshot(source,native);
        same(source.SQUALL_Listener_SetVelocity(value.data()),native.SQUALL_Listener_SetVelocity(value.data()),"SetVelocity patterns");++setterCalls;snapshot(source,native);
        for(unsigned mask=0;mask<16;++mask){
            std::array<float*,4> input{};for(unsigned i=0;i<4;++i)input[i]=(mask&(1U<<i)) ? value.data():nullptr;
            same(source.SQUALL_Listener_SetParameters(input[0],input[1],input[2],input[3]),native.SQUALL_Listener_SetParameters(input[0],input[1],input[2],input[3]),"Combined vector pattern/null mask");++setterCalls;snapshot(source,native);
        }
        same(source.SQUALL_Listener_Update(),native.SQUALL_Listener_Update(),"Actual deferred commit");snapshot(source,native);
    }
    const std::array<float*,4> bad{nullptr,reinterpret_cast<float*>(std::uintptr_t(1)),pages.cross(),pages.readOnly()};
    for(unsigned n=0;n<bad.size();++n){
        caseId=n;auto p=bad[n];same(source.SQUALL_Listener_SetPosition(p),native.SQUALL_Listener_SetPosition(p),"SetPosition input pages");++setterCalls;snapshot(source,native);
        same(source.SQUALL_Listener_SetVelocity(p),native.SQUALL_Listener_SetVelocity(p),"SetVelocity input pages");++setterCalls;snapshot(source,native);
        same(source.SQUALL_Listener_SetParameters(p,p,p,p),native.SQUALL_Listener_SetParameters(p,p,p,p),"Combined input pages");++setterCalls;snapshot(source,native);
    }
    getters(source,native,pages);
}
}
int wmain(int argc,wchar_t** argv){
    if(argc<3||argc>4)return 2;HWND window=nullptr;
    try{
        Api native(argv[1]),source(argv[2]);_control87(argc==4 ? _PC_24:_PC_53,_MCW_PC);Pages pages;
        exercise(source,native,pages);
        window=CreateWindowExW(0,L"STATIC",L"Squall actual listener vectors",0,0,0,64,64,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);require(window!=nullptr,"CreateWindow failed");
        for(int deferred=0;deferred<2;++deferred){
            contextId=deferred+1;squall_parameters_t parameters{};parameters.Window=window;parameters.SampleRate=44100;parameters.BitPerSample=16;parameters.Channels=16;parameters.UseAlg=3;parameters.BufferSize=200;parameters.ListenerMode=deferred;parameters.DistanceFactor=1;parameters.RolloffFactor=.001f;parameters.DopplerFactor=1;
            const auto initialized=native.SQUALL_Init(&parameters);same(source.SQUALL_Init(&parameters),initialized,"Actual Init");require(initialized>=0,"Actual listener Init failed");exercise(source,native,pages);source.SQUALL_Free();native.SQUALL_Free();
        }
        contextId=3;exercise(source,native,pages);
        require((_control87(0,0)&_MCW_PC)==(argc==4 ? _PC_24:_PC_53),"x87 precision changed");DestroyWindow(window);window=nullptr;
        std::cout<<"{\"result_comparisons\":"<<comparisons<<",\"snapshots\":"<<snapshots<<",\"setter_calls\":"<<setterCalls<<",\"getter_pointer_cases\":"<<getterCases<<",\"actual_directsound\":true,\"contexts\":4,\"export_lookups\":194}\n";return 0;
    }catch(const std::exception& error){if(window)DestroyWindow(window);std::cerr<<error.what()<<'\n';return 1;}
}
