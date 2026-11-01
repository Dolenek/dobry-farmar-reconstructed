#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "Squall.h"
#include <array>
#include <vector>
#include <filesystem>
#include <float.h>
#include <cstdint>
#include <iostream>
#include <stdexcept>
namespace {
void need(bool value,const char* text){if(!value)throw std::runtime_error(text);}
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
    ~Api(){if(module){SQUALL_Free();SQUALL_Sample_UnloadAll();FreeLibrary(module);}}
};
unsigned comparisons{},pointerCases{},created{},observations{};
void same(int a,int b,const char* name){++comparisons;if(a!=b){std::cerr<<name<<" source="<<a<<" native="<<b<<'\n';throw std::runtime_error("Channel worker API differs");}}
struct User {
    volatile LONG calls{},bad{};
    Api* api{};int channel{},stage{},result=1;DWORD mainThread=GetCurrentThreadId();std::uint32_t magic=0x17395186;
};
int __cdecl worker(int channel,void* data){
    auto* user=static_cast<User*>(data);
    if(user->magic!=0x17395186||channel!=user->channel||GetCurrentThreadId()==user->mainThread||user->api->SQUALL_Channel_Status(channel)!=user->stage)InterlockedIncrement(&user->bad);
    InterlockedIncrement(&user->calls);return user->result;
}
const std::array<unsigned,8> periods{0,1,10,100,0x7fffffff,0xffffffff,0xfffffffe,0x80000000};
void pointers(Api& s,Api& n,int sc,int nc,int stage){
    User own,original;own.api=&s;own.channel=sc;own.stage=stage;original.api=&n;original.channel=nc;original.stage=stage;
    for(auto period:periods){
        same(s.SQUALL_Channel_SetWorker(sc,nullptr,&own,period),n.SQUALL_Channel_SetWorker(nc,nullptr,&original,period),"Null channel worker");++pointerCases;
        same(s.SQUALL_Channel_SetWorker(sc,&worker,&own,period),n.SQUALL_Channel_SetWorker(nc,&worker,&original,period),"Valid channel worker");++pointerCases;
        same(s.SQUALL_Channel_SetWorker(sc,reinterpret_cast<squall_callback_channel_t>(1),&own,period),n.SQUALL_Channel_SetWorker(nc,reinterpret_cast<squall_callback_channel_t>(1),&original,period),"Invalid channel worker");++pointerCases;
    }
    same(s.SQUALL_Channel_SetWorker(sc,nullptr,nullptr,0),n.SQUALL_Channel_SetWorker(nc,nullptr,nullptr,0),"Clear channel worker");
    need(own.bad==0&&original.bad==0,"Setter matrix callback data mismatch");
}
void empty(Api& s,Api& n){for(int id:{0,-1,0x7fffffff,0x10000})pointers(s,n,id,id,0);}
int make(Api& api,int sample,bool spatial,int stage){
    std::array<float,3> zero{};
    int channel=spatial?api.SQUALL_Sample_Play3DEx(sample,1,17,stage==3?0:1,zero.data(),zero.data(),7,0,0,1,1000):api.SQUALL_Sample_PlayEx(sample,1,17,stage==3?0:1,7,0,0,50);
    need(channel>=0,"Create actual worker channel");++created;
    if(stage==2)need(api.SQUALL_Channel_Pause(channel,1)>=0,"Pause worker channel");need(api.SQUALL_Channel_Status(channel)==stage,"Worker channel stage mismatch");return channel;
}
void retire(Api& api,int channel){
    need(api.SQUALL_Channel_Stop(channel)>=0,"Stop worker channel");const auto end=GetTickCount64()+2000;
    while(api.SQUALL_Channel_Status(channel)!=0){need(GetTickCount64()<end,"Worker channel not retired");Sleep(1);}
}
struct Observation {unsigned dll,spatial,stage,period,calls,retired;};
std::vector<Observation> traces;
void retained(Api& api,unsigned dll,int sample,bool spatial,int stage){
    int channel=make(api,sample,spatial,stage);User user;user.api=&api;user.channel=channel;user.stage=stage;User rejected=user;
    need(api.SQUALL_Channel_SetWorker(channel,&worker,&user,10)>=0,"Install channel worker");
    need(api.SQUALL_Channel_SetWorker(channel,reinterpret_cast<squall_callback_channel_t>(1),&rejected,10)==-4,"Bad worker not rejected");
    need(api.SQUALL_Channel_SetWorker(channel,&worker,&rejected,0)==-4,"Zero worker period not rejected");
    const auto deadline=GetTickCount64()+1500;while(user.calls<2){need(GetTickCount64()<deadline,"Actual worker did not call");Sleep(2);}
    need(api.SQUALL_Channel_SetWorker(channel,nullptr,nullptr,0)>=0,"Clear active worker");const auto observed=user.calls;
    need(user.bad==0&&rejected.calls==0,"Worker ID/data/thread/reentrant getter or retention failed");Sleep(35);need(user.calls==observed,"Cleared channel worker called again");
    traces.push_back({dll,unsigned(spatial),unsigned(stage),10,unsigned(observed),0});++observations;retire(api,channel);
}
void returnZero(Api& api,unsigned dll,int sample,bool spatial){
    int channel=make(api,sample,spatial,3);User user;user.api=&api;user.channel=channel;user.stage=3;user.result=0;
    need(api.SQUALL_Channel_SetWorker(channel,&worker,&user,1)>=0,"Install retiring callback");const auto deadline=GetTickCount64()+1500;
    while(api.SQUALL_Channel_Status(channel)!=0){need(GetTickCount64()<deadline,"Zero callback did not delete channel");Sleep(2);}
    need(user.calls==1&&user.bad==0,"Zero callback called wrong number of times or with invalid context");Sleep(35);need(user.calls==1,"Retired worker called again");
    traces.push_back({dll,unsigned(spatial),3,1,1,1});++observations;
}
void highPeriods(Api& api,unsigned dll,int sample){
    for(auto period:{0x7fffffffU,0xffffffffU,0xfffffffeU,0x80000000U}){
        int channel=make(api,sample,false,3);User user;user.api=&api;user.channel=channel;user.stage=3;
        need(api.SQUALL_Channel_SetWorker(channel,&worker,&user,period)>=0,"High-bit channel period rejected");Sleep(35);
        need(api.SQUALL_Channel_SetWorker(channel,nullptr,nullptr,0)>=0,"Clear high-bit worker");need(user.bad==0,"High-bit worker has wrong context");
        // Native channel worker uses a signed comparison, unlike listener.
        need(period==0x7fffffffU?user.calls==0:user.calls>0,"Signed channel-worker timing differs");
        traces.push_back({dll,0,3,period,unsigned(user.calls),0});++observations;retire(api,channel);
    }
}
}
int wmain(int argc,wchar_t** argv){
    if(argc<3||argc>4)return 2;HWND window{};
    try{
        Api native(argv[1]),source(argv[2]);const auto precision=argc==4?_PC_24:_PC_53;_control87(precision,_MCW_PC);empty(source,native);
        char filename[]="sample.wav";int ns=native.SQUALL_Sample_LoadFile(filename,1,nullptr),ss=source.SQUALL_Sample_LoadFile(filename,1,nullptr);same(ss,ns,"Load actual PCM");need(ns>0,"PCM load failed");
        window=CreateWindowExW(0,L"STATIC",L"Squall channel worker",0,0,0,64,64,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);need(window!=nullptr,"Create audio window");
        squall_parameters_t p{};p.Window=window;p.SampleRate=44100;p.BitPerSample=16;p.Channels=4;p.UseAlg=1;p.BufferSize=200;p.DistanceFactor=1;p.RolloffFactor=.001f;p.DopplerFactor=1;
        auto result=native.SQUALL_Init(&p);same(source.SQUALL_Init(&p),result,"DirectSound Init");need(result>=0,"Actual Init failed");
        auto nc=make(native,ns,false,3),sc=make(source,ss,false,3);same(sc,nc,"Prepared callback channel");pointers(source,native,sc,nc,3);retire(source,sc);retire(native,nc);
        for(bool spatial:{false,true})for(int stage:{3,1,2}){retained(native,0,ns,spatial,stage);retained(source,1,ss,spatial,stage);}
        for(bool spatial:{false,true}){returnZero(native,0,ns,spatial);returnZero(source,1,ss,spatial);}
        highPeriods(native,0,ns);highPeriods(source,1,ss);
        source.SQUALL_Free();native.SQUALL_Free();empty(source,native);need((_control87(0,0)&_MCW_PC)==precision,"Precision changed");DestroyWindow(window);window=nullptr;
        std::cout<<"{\"result_comparisons\":"<<comparisons<<",\"pointer_cases\":"<<pointerCases<<",\"actual_created_channels\":"<<created<<",\"observations\":"<<observations<<",\"export_lookups\":194,\"actual_directsound\":true,\"callback_observations\":[";
        for(unsigned i=0;i<traces.size();++i){auto t=traces[i];if(i)std::cout<<',';std::cout<<'['<<t.dll<<','<<t.spatial<<','<<t.stage<<','<<t.period<<','<<t.calls<<','<<t.retired<<']';}std::cout<<"]}\n";return 0;
    }catch(const std::exception& error){if(window)DestroyWindow(window);std::cerr<<error.what()<<'\n';return 1;}
}
