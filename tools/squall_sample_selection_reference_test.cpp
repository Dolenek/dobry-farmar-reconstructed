// Real sample-group APIs plus original/source selector calls for cap coverage.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include "Squall.h"
#include "SoundEngine/Context.h"
#include <array>
#include <vector>
#include <filesystem>
#include <float.h>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
namespace {
void need(bool ok,const char* text){if(!ok)throw std::runtime_error(text);}
struct Api {
#define SQUALL_ENTRY(name) decltype(&name) name{};
#include "squall_exports.inc"
#undef SQUALL_ENTRY
    HMODULE module{};context_t* context{};int(__cdecl* select)(int,unsigned){};
    Api(const wchar_t* path,unsigned selector,unsigned contextRva){module=LoadLibraryW(std::filesystem::absolute(path).c_str());need(module!=nullptr,"Load DLL");
#define SQUALL_ENTRY(name) name=reinterpret_cast<decltype(name)>(GetProcAddress(module,#name));need(name!=nullptr,#name);
#include "squall_exports.inc"
#undef SQUALL_ENTRY
        select=reinterpret_cast<decltype(select)>(reinterpret_cast<std::uintptr_t>(module)+selector);context=reinterpret_cast<context_t*>(reinterpret_cast<std::uintptr_t>(module)+contextRva);
    }
    ~Api(){if(module){SQUALL_Free();SQUALL_Sample_UnloadAll();FreeLibrary(module);}}
};
static_assert(sizeof(SSample)==300 && offsetof(SSample,SampleID)==0x4c && offsetof(context_t,_channelsArray)==0x100,"Native read-only channel identity");
unsigned results{},selections{},actualChannels{};std::uint32_t expectedSeed=1;
void same(int a,int b,const char* text){++results;if(a!=b){std::cerr<<text<<" source="<<a<<" native="<<b<<'\n';throw std::runtime_error("Sample-group selection differs");}}
unsigned next(unsigned count){expectedSeed=expectedSeed*0x343fdU+0x269ec3U;return ((expectedSeed>>16)&32767U)%count;}
std::array<int,300> sourceSamples{},nativeSamples{};
squall_sample_default_t defaults(unsigned i,int group){squall_sample_default_t p{};p.SampleGroupID=group;p.Priority=7;p.Frequency=8000+int(i)*80;p.Volume=0;p.Pan=39;p.MinDist=1;p.MaxDist=1000;return p;}
void stop(Api& api,int c){need(api.SQUALL_Channel_Stop(c)>=0,"Stop selected channel");const auto end=GetTickCount64()+2000;while(api.SQUALL_Channel_Status(c)!=0){need(GetTickCount64()<end,"Selection channel did not retire");Sleep(1);}}
int play(Api& api,unsigned method){std::array<float,3> zero{};
    switch(method){
    case 0:return api.SQUALL_SampleGroup_Play(73,1,17,0);
    case 1:return api.SQUALL_SampleGroup_PlayEx(73,1,17,0,19,0,0,57);
    case 2:return api.SQUALL_SampleGroup_Play3D(73,1,17,0,zero.data(),zero.data());
    default:return api.SQUALL_SampleGroup_Play3DEx(73,1,17,0,zero.data(),zero.data(),19,0,0,1,1000);
    }
}
struct Summary {unsigned members,eligible;std::uint64_t sequence=14695981039346656037ULL;std::vector<unsigned> histogram;};
std::vector<Summary> summaries;
}
int wmain(int argc,wchar_t** argv){
    if(argc<5||argc>6)return 2;HWND window{};
    try{
        Api native(argv[1],0x8330,0x75034),source(argv[2],std::stoul(argv[3],nullptr,16),std::stoul(argv[4],nullptr,16));const auto precision=argc==6?_PC_24:_PC_53;_control87(precision,_MCW_PC);
        for(unsigned i=0;i<300;++i){char filename[]="sample.wav";auto p=defaults(i,74);nativeSamples[i]=native.SQUALL_Sample_LoadFile(filename,1,&p);sourceSamples[i]=source.SQUALL_Sample_LoadFile(filename,1,&p);same(sourceSamples[i],nativeSamples[i],"Load shared-file independent sample");need(nativeSamples[i]>0,"Load group sample failed");}
        window=CreateWindowExW(0,L"STATIC",L"Squall multi-member selection",0,0,0,64,64,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);need(window!=nullptr,"Create actual window");
        squall_parameters_t p{};p.Window=window;p.SampleRate=44100;p.BitPerSample=16;p.Channels=4;p.UseAlg=1;p.BufferSize=200;p.DistanceFactor=1;p.RolloffFactor=.001f;p.DopplerFactor=1;
        auto initialized=native.SQUALL_Init(&p);same(source.SQUALL_Init(&p),initialized,"Actual DirectSound Init");need(initialized>=0,"Actual DirectSound Init failed");
        for(unsigned count:{1U,2U,7U,255U,256U,257U,300U}){
            for(unsigned i=0;i<300;++i){auto def=defaults(i,i<count?73:74);same(source.SQUALL_Sample_SetDefault(sourceSamples[i],&def),native.SQUALL_Sample_SetDefault(nativeSamples[i],&def),"Actual group membership");}
            const auto eligible=(std::min)(count,256U);Summary summary{count,eligible};summary.histogram.assign(eligible,0);
            for(unsigned i=0;i<16384;++i){
                if(i%127==0){same(source.select(123456,~i),native.select(123456,~i),"Missing group selector");need(native.select(123456,0)==0,"Empty selector result");}
                const unsigned unused=i%4==0?0:i%4==1?1:i%4==2?0xffffffffU:256;
                const auto b=native.select(73,unused),a=source.select(73,unused);same(a,b,"Actual selector integer ID");++selections;
                const auto index=next(eligible);need(b==nativeSamples[index],"Native selection differs from untouched default CRT sequence/cap");
                ++summary.histogram[index];summary.sequence^=unsigned(b);summary.sequence*=1099511628211ULL;
            }
            for(auto frequency:summary.histogram)need(frequency>0,"Not every eligible member was selected");
            for(unsigned method=0;method<4;++method)for(unsigned i=0;i<4;++i){
                const auto nc=play(native,method),sc=play(source,method);same(sc,nc,"Real multi-member export channel");need(nc>=0,"Actual group playback failed");++actualChannels;
                const auto index=next(eligible);const auto ns=native.context->_channelsArray[unsigned(nc)&255].SampleID,ss=source.context->_channelsArray[unsigned(sc)&255].SampleID;
                same(ss,ns,"Read-only actual selected sample ID");need(ns==nativeSamples[index],"Export did not use expected actual member");
                same(source.SQUALL_Channel_GetFrequency(sc),native.SQUALL_Channel_GetFrequency(nc),"Actual selected frequency");
                need(native.SQUALL_Channel_GetFrequency(nc)==(method==0||method==2?8000+int(index)*80:8000),"Default/explicit selection frequency differs");
                same(source.SQUALL_Channel_GetPriority(sc),native.SQUALL_Channel_GetPriority(nc),"Actual selected priority");
                need(native.SQUALL_Channel_GetPriority(nc)==(method==0||method==2?7:19),"Default/explicit selection priority differs");
                stop(source,sc);stop(native,nc);
            }
            summaries.push_back(summary);
        }
        source.SQUALL_Sample_UnloadAll();native.SQUALL_Sample_UnloadAll();same(source.select(73,0),native.select(73,0),"Unloaded group");need(native.select(73,0)==0,"Unloaded group still selects");
        need((_control87(0,0)&_MCW_PC)==precision,"Precision changed");source.SQUALL_Free();native.SQUALL_Free();DestroyWindow(window);window=nullptr;
        std::cout<<"{\"result_comparisons\":"<<results<<",\"selector_calls_compared\":"<<selections<<",\"actual_export_channels\":"<<actualChannels<<",\"loaded_samples_per_dll\":300,\"original_selector_rva\":33584,\"rng_state_written\":false,\"private_context_read_only\":true,\"actual_directsound\":true,\"export_lookups\":194,\"groups\":[";
        for(unsigned i=0;i<summaries.size();++i){const auto& s=summaries[i];if(i)std::cout<<',';std::cout<<"{\"members\":"<<s.members<<",\"eligible\":"<<s.eligible<<",\"sequence_fnv1a64\":\""<<std::hex<<s.sequence<<std::dec<<"\",\"selected_counts\":[";for(unsigned j=0;j<s.histogram.size();++j){if(j)std::cout<<',';std::cout<<s.histogram[j];}std::cout<<"]}";}std::cout<<"]}\n";return 0;
    }catch(const std::exception& error){if(window)DestroyWindow(window);std::cerr<<error.what()<<'\n';return 1;}
}
