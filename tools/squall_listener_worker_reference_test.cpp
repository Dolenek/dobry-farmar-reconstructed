#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "Squall.h"
#include <array>
#include <filesystem>
#include <float.h>
#include <cstdint>
#include <iostream>
#include <stdexcept>
namespace {
void need(bool ok,const char* text){if(!ok)throw std::runtime_error(text);}
struct Api {
#define SQUALL_ENTRY(name) decltype(&name) name{};
#include "squall_exports.inc"
#undef SQUALL_ENTRY
    HMODULE module{};
    explicit Api(const wchar_t* path){module=LoadLibraryW(std::filesystem::absolute(path).c_str());need(module!=nullptr,"LoadLibrary failed");
#define SQUALL_ENTRY(name) name=reinterpret_cast<decltype(name)>(GetProcAddress(module,#name));need(name!=nullptr,#name);
#include "squall_exports.inc"
#undef SQUALL_ENTRY
    }
    ~Api(){if(module){SQUALL_Listener_SetWorker(nullptr,nullptr,0);SQUALL_Free();FreeLibrary(module);}}
};
unsigned comparisons=0,mismatches=0;bool diagnose=false;
void same(int a,int b,const char* label,int period){++comparisons;if(a!=b){++mismatches;if(mismatches<9)std::cerr<<label<<" period="<<period<<" source="<<a<<" native="<<b<<'\n';if(!diagnose)throw std::runtime_error("Listener worker differs");}}
struct User {volatile LONG calls=0,bad=0;std::uint32_t magic=0x17395186;};
void __cdecl worker(void* data){auto* user=static_cast<User*>(data);if(user->magic!=0x17395186)InterlockedIncrement(&user->bad);InterlockedIncrement(&user->calls);}
void cases(Api& source,Api& native,void* data){
    for(int period:{0,1,10,100,0x7fffffff,-1,-2,static_cast<int>(0x80000000U)}){
        same(source.SQUALL_Listener_SetWorker(nullptr,data,period),native.SQUALL_Listener_SetWorker(nullptr,data,period),"Null worker",period);
        same(source.SQUALL_Listener_SetWorker(&worker,data,period),native.SQUALL_Listener_SetWorker(&worker,data,period),"Valid worker",period);
        same(source.SQUALL_Listener_SetWorker(reinterpret_cast<squall_callback_listener_t>(std::uintptr_t(1)),data,period),native.SQUALL_Listener_SetWorker(reinterpret_cast<squall_callback_listener_t>(std::uintptr_t(1)),data,period),"Invalid code pointer",period);
    }
    same(source.SQUALL_Listener_SetWorker(nullptr,nullptr,0),native.SQUALL_Listener_SetWorker(nullptr,nullptr,0),"Clear worker",0);
}
unsigned actualCallbacks(Api& api,User& user){
    user.calls=0;user.bad=0;need(api.SQUALL_Listener_SetWorker(&worker,&user,10)>=0,"Install actual worker");
    User rejected;
    need(api.SQUALL_Listener_SetWorker(reinterpret_cast<squall_callback_listener_t>(std::uintptr_t(1)),&rejected,10)==-4,"Invalid code pointer accepted");
    need(api.SQUALL_Listener_SetWorker(&worker,&rejected,0)==-4,"Zero period accepted");
    const auto started=GetTickCount64();while(user.calls<2&&GetTickCount64()-started<1500)Sleep(5);
    need(api.SQUALL_Listener_SetWorker(nullptr,nullptr,0)>=0,"Clear actual worker");const auto observed=user.calls;need(observed>=2&&user.bad==0&&rejected.calls==0,"Actual worker callback, userdata or rejected-setter retention failed");Sleep(35);need(user.calls==observed,"Cleared worker still called");return unsigned(observed);
}
}
int wmain(int argc,wchar_t** argv){
    if(argc<3||argc>4)return 2;HWND window=nullptr;
    try{
        Api native(argv[1]),source(argv[2]);_control87(argc==4 ? _PC_24:_PC_53,_MCW_PC);diagnose=GetEnvironmentVariableW(L"FARMAR_LISTENER_WORKER_DIAGNOSE",nullptr,0)!=0;
        User shared;cases(source,native,&shared);
        if(diagnose){std::cout<<"{\"preinit_comparisons\":"<<comparisons<<",\"mismatches\":"<<mismatches<<"}\n";return mismatches ? 1:0;}
        window=CreateWindowExW(0,L"STATIC",L"Squall actual listener worker",0,0,0,64,64,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);need(window!=nullptr,"Create actual window");
        squall_parameters_t p{};p.Window=window;p.SampleRate=44100;p.BitPerSample=16;p.Channels=16;p.UseAlg=3;p.BufferSize=200;p.DistanceFactor=1;p.RolloffFactor=.001f;p.DopplerFactor=1;
        const auto initialized=native.SQUALL_Init(&p);same(source.SQUALL_Init(&p),initialized,"Actual Init",0);need(initialized>=0,"Init failed");
        cases(source,native,&shared);User originalUser,sourceUser;const auto originalCalls=actualCallbacks(native,originalUser),ownCalls=actualCallbacks(source,sourceUser);
        // Negative int periods are accepted DWORD bit patterns, with no callback
        // within this short observation window; this is not a49-day timer test.
        for(Api* api:{&source,&native})for(int period:{-1,-2,static_cast<int>(0x80000000U)}){
            User user;need(api->SQUALL_Listener_SetWorker(&worker,&user,period)>=0,"Negative DWORD period rejected");Sleep(35);need(api->SQUALL_Listener_SetWorker(nullptr,nullptr,0)>=0,"Clear negative worker");need(user.calls==0,"Huge unsigned period fired in short window");
        }
        source.SQUALL_Free();native.SQUALL_Free();cases(source,native,&shared);
        need((_control87(0,0)&_MCW_PC)==(argc==4 ? _PC_24:_PC_53),"x87 precision changed");DestroyWindow(window);window=nullptr;
        std::cout<<"{\"result_comparisons\":"<<comparisons<<",\"native_callbacks\":"<<originalCalls<<",\"source_callbacks\":"<<ownCalls<<",\"negative_period_observations\":6,\"actual_directsound\":true,\"export_lookups\":194}\n";return 0;
    }catch(const std::exception& error){if(window)DestroyWindow(window);std::cerr<<error.what()<<'\n';return 1;}
}
