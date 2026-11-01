// Actual DLL error paths on the available host; supported hardware paths are separate.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "Squall.h"
#include <array>
#include <map>
#include <vector>
#include <filesystem>
#include <float.h>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
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
    ~Api(){if(module){SQUALL_Free();SQUALL_Sample_UnloadAll();FreeLibrary(module);}}
};
unsigned comparisons{},snapshots{},phase{},method{},mode{},versionBits{};
int channelCase{};
std::array<std::map<int,unsigned>,7> histograms;
void same(int a,int b,const char* name){++comparisons;if(a!=b){std::cerr<<name<<" phase="<<phase<<" method="<<method<<" mode="<<mode<<" version="<<versionBits<<" channel="<<channelCase<<" source="<<a<<" native="<<b<<'\n';throw std::runtime_error("Extension API differs");}}
void* pointer(unsigned mode,std::array<unsigned char,256>& words,char* pages,unsigned size){
    if(mode==0)return words.data()+16;if(mode==1)return nullptr;if(mode==2)return reinterpret_cast<void*>(1);
    if(mode==3)return pages;if(mode==4)return pages+8192-size+1;return pages+8192;
}
int call(Api& api,unsigned method,int channel,int version,void* p){
    switch(method){
    case 0:return api.SQUALL_Listener_EAX_SetProperties(version,static_cast<squall_eax_listener_t*>(p));
    case 1:return api.SQUALL_Listener_EAX_GetProperties(version,static_cast<squall_eax_listener_t*>(p));
    case 2:return api.SQUALL_Channel_EAX_SetProperties(channel,version,static_cast<squall_eax_channel_t*>(p));
    case 3:return api.SQUALL_Channel_EAX_GetProperties(channel,version,static_cast<squall_eax_channel_t*>(p));
    case 4:return api.SQUALL_Channel_ZOOMFX_SetProperties(channel,static_cast<squall_zoomfx_channel_t*>(p));
    default:return api.SQUALL_Channel_ZOOMFX_GetProperties(channel,static_cast<squall_zoomfx_channel_t*>(p));
    }
}
void cases(Api& s,Api& n,int sc=-1,int nc=-1){
    auto* pages=static_cast<char*>(VirtualAlloc(nullptr,12288,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));need(pages!=nullptr,"Allocate protected pages");std::memset(pages,0x5a,8192);
    DWORD old{};need(VirtualProtect(pages,4096,PAGE_READONLY,&old)!=FALSE,"Read-only property page");need(VirtualProtect(pages+8192,4096,PAGE_NOACCESS,&old)!=FALSE,"No-access property page");
    for(method=0;method<6;++method)for(int version:{INT_MIN,-1,0,1,2,3,4,INT_MAX})for(mode=0;mode<6;++mode){
        versionBits=unsigned(version);if(method>=4&&version!=0)continue;
        const unsigned size=method<2?sizeof(squall_eax_listener_t):method<4?sizeof(squall_eax_channel_t):sizeof(squall_zoomfx_channel_t);
        for(unsigned id=0;id<(method<2?1U:5U);++id){
            const std::array<int,4> invalid{0,-1,0x7fffffff,0x10000};const int aId=id?invalid[id-1]:sc,bId=id?invalid[id-1]:nc;channelCase=bId;
            std::array<unsigned char,256>a,b;a.fill(0x5a);b=a;const auto initial=a;
            const auto nr=call(n,method,bId,version,pointer(mode,b,pages,size));const auto sr=call(s,method,aId,version,pointer(mode,a,pages,size));same(sr,nr,"Actual extension properties");++histograms[method][nr];
            need(a==b,"Extension property words/guards differ");
            // This host has no EAX or ZOOMFX: errors must leave full output unchanged.
            need(a==initial,"Unsupported extension changed caller memory");++snapshots;
        }
    }
    method=6;
    for(int preset:{INT_MIN,-2,-1,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,INT_MAX}){
        const auto result=n.SQUALL_Listener_EAX_SetPreset(preset);same(s.SQUALL_Listener_EAX_SetPreset(preset),result,"Actual EAX preset");++histograms[6][result];}
    VirtualFree(pages,0,MEM_RELEASE);
}
void retire(Api& api,int channel){need(api.SQUALL_Channel_Stop(channel)>=0,"Stop extension channel");const auto end=GetTickCount64()+2000;while(api.SQUALL_Channel_Status(channel)!=0){need(GetTickCount64()<end,"Extension worker did not retire");Sleep(1);}}
}
int wmain(int argc,wchar_t** argv){
    if(argc<3||argc>4)return 2;HWND window{};
    try{
        Api native(argv[1]),source(argv[2]);const auto precision=argc==4?_PC_24:_PC_53;_control87(precision,_MCW_PC);
        same(source.SQUALL_GetEAXVersion(),native.SQUALL_GetEAXVersion(),"PreInit EAX version");need(native.SQUALL_GetEAXVersion()==0,"This scope requires actual no-EAX host");cases(source,native);
        char filename[]="sample.wav";auto ns=native.SQUALL_Sample_LoadFile(filename,1,nullptr),ss=source.SQUALL_Sample_LoadFile(filename,1,nullptr);same(ss,ns,"PCM load");need(ns>0,"Actual PCM load failed");
        window=CreateWindowExW(0,L"STATIC",L"Squall actual extensions",0,0,0,64,64,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);need(window!=nullptr,"Create audio window");
        squall_parameters_t p{};p.Window=window;p.SampleRate=44100;p.BitPerSample=16;p.Channels=4;p.UseAlg=1;p.BufferSize=200;p.DistanceFactor=1;p.RolloffFactor=.001f;p.DopplerFactor=1;
        const auto result=native.SQUALL_Init(&p);same(source.SQUALL_Init(&p),result,"Actual DirectSound Init");need(result>=0,"Actual Init failed");
        same(source.SQUALL_GetEAXVersion(),native.SQUALL_GetEAXVersion(),"Actual host EAX version");need(native.SQUALL_GetEAXVersion()==0,"Supported EAX needs its separate hardware scope");
        phase=1;cases(source,native);phase=2;
        auto nc=native.SQUALL_Sample_PlayEx(ns,1,17,0,7,0,0,50),sc=source.SQUALL_Sample_PlayEx(ss,1,17,0,7,0,0,50);same(sc,nc,"Actual2D channel");need(nc>=0,"Actual2D creation failed");cases(source,native,sc,nc);retire(source,sc);retire(native,nc);
        phase=3;std::array<float,3>zero{};nc=native.SQUALL_Sample_Play3DEx(ns,1,17,0,zero.data(),zero.data(),7,0,0,1,1000);sc=source.SQUALL_Sample_Play3DEx(ss,1,17,0,zero.data(),zero.data(),7,0,0,1,1000);same(sc,nc,"Actual3D channel");need(nc>=0,"Actual3D creation failed");cases(source,native,sc,nc);retire(source,sc);retire(native,nc);
        source.SQUALL_Free();native.SQUALL_Free();phase=4;cases(source,native);need((_control87(0,0)&_MCW_PC)==precision,"Precision changed");DestroyWindow(window);window=nullptr;
        std::cout<<"{\"result_comparisons\":"<<comparisons<<",\"guarded_property_snapshots\":"<<snapshots<<",\"export_lookups\":194,\"actual_directsound\":true,\"actual_eax_version\":0,\"successful_hardware_extension_paths_verified\":false,\"listener_bytes\":"<<sizeof(squall_eax_listener_t)<<",\"channel_bytes\":"<<sizeof(squall_eax_channel_t)<<",\"zoomfx_bytes\":"<<sizeof(squall_zoomfx_channel_t)<<",\"result_histograms\":[";
        for(unsigned i=0;i<histograms.size();++i){if(i)std::cout<<',';std::cout<<'[';unsigned count=0;for(auto [result,cases]:histograms[i]){if(count++)std::cout<<',';std::cout<<'['<<result<<','<<cases<<']';}std::cout<<']';}std::cout<<"]}\n";return 0;
    }catch(const std::exception& error){if(window)DestroyWindow(window);std::cerr<<error.what()<<'\n';return 1;}
}
