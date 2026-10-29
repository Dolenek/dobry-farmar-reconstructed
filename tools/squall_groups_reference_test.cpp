// Compare actual public group APIs in two independently loaded DirectSound
// implementations. No original instructions or private state are substituted.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "Squall.h"
#include "SoundFiles.h"
#include "StringConversion.h"
#include <array>
#include <filesystem>
#include <float.h>
#include <iostream>
#include <limits>
#include <stdexcept>
using namespace farmar::recovered;
namespace {
struct Api {
#define SQUALL_ENTRY(name) decltype(&name) name{};
#include "squall_exports.inc"
#undef SQUALL_ENTRY
    HMODULE module{};
    explicit Api(const wchar_t* filename) {
        module=LoadLibraryW(std::filesystem::absolute(filename).c_str());
        if(!module) throw std::runtime_error("LoadLibrary failed");
#define SQUALL_ENTRY(name) name=reinterpret_cast<decltype(name)>(GetProcAddress(module,#name));if(!name) throw std::runtime_error(#name);
#include "squall_exports.inc"
#undef SQUALL_ENTRY
    }
    ~Api(){if(module){SQUALL_Free();FreeLibrary(module);}}
};
void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
unsigned comparisons=0,emptyCalls=0,retirePolls=0;
void same(int actual,int expected,const char* operation){
    ++comparisons;
    if(actual!=expected){std::cerr<<operation<<": source="<<actual<<" native="<<expected<<'\n';throw std::runtime_error("Public group API mismatch");}
}
void emptyGroups(Api& source,Api& native){
    for(int group:{17,0,-1,(std::numeric_limits<int>::max)()}){
        for(int pause:{0,1,-1,2}){
            same(source.SQUALL_ChannelGroup_Pause(group,pause),native.SQUALL_ChannelGroup_Pause(group,pause),"Empty group pause");++emptyCalls;
        }
        same(source.SQUALL_ChannelGroup_Stop(group),native.SQUALL_ChannelGroup_Stop(group),"Empty group stop");++emptyCalls;
        // With no live channels the original never indexes its volume table.
        // These inputs do not exercise an out-of-bounds volume-table read.
        for(int volume:{-1,(std::numeric_limits<int>::min)(),0,255,256,(std::numeric_limits<int>::max)()}){
            same(source.SQUALL_ChannelGroup_SetVolume(group,volume),native.SQUALL_ChannelGroup_SetVolume(group,volume),"Empty group volume");++emptyCalls;
        }
        for(int frequency:{-1,0,99,100,100000,100001,(std::numeric_limits<int>::max)()}){
            same(source.SQUALL_ChannelGroup_SetFrequency(group,frequency),native.SQUALL_ChannelGroup_SetFrequency(group,frequency),"Empty group frequency");++emptyCalls;
        }
    }
}
}
int wmain(int argc,wchar_t** argv){
    if(argc<4 || argc>5)return 2;
    HWND window=nullptr;
    try{
        FileIndex files(toNarrow(WString(argv[3])),String("pack"));files.mountCommon(toNarrow(WString(argv[3])));
        SoundFiles callbacks(files);Api native(argv[1]),source(argv[2]);
        _control87(argc==5 ? _PC_24:_PC_53,_MCW_PC);
        const int devices=native.SQUALL_GetNumDevice();same(source.SQUALL_GetNumDevice(),devices,"Device count");require(devices>0,"Real audio device required");
        emptyGroups(source,native);
        window=CreateWindowExW(0,L"STATIC",L"Squall group comparison",0,0,0,64,64,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);
        require(window!=nullptr,"Real hidden window required");
        squall_parameters_t parameters{};parameters.Window=window;parameters.SampleRate=44100;parameters.BitPerSample=16;parameters.Channels=16;
        parameters.UseAlg=3;parameters.BufferSize=200;parameters.DistanceFactor=1;parameters.RolloffFactor=.001f;parameters.DopplerFactor=1;
        const int initialized=native.SQUALL_Init(&parameters);same(source.SQUALL_Init(&parameters),initialized,"Init");require(initialized>=0,"Real DirectSound initialization failed");
        emptyGroups(source,native);
        same(source.SQUALL_SetFileCallbacks(&SoundFiles::open,&SoundFiles::seek,&SoundFiles::read,&SoundFiles::close),native.SQUALL_SetFileCallbacks(&SoundFiles::open,&SoundFiles::seek,&SoundFiles::read,&SoundFiles::close),"File callbacks");
        std::string filename="sounds/menu_music.ogg";
        const int ns=native.SQUALL_Sample_LoadFile(filename.data(),1,nullptr),ss=source.SQUALL_Sample_LoadFile(filename.data(),1,nullptr);
        same(ss,ns,"Actual sample load");require(ns>0,"Actual menu sample failed");
        std::array<int,3> nc{},sc{};
        for(unsigned i=0;i<3;++i){
            const int group=i<2 ? 17:-7;
            nc[i]=native.SQUALL_Sample_PlayEx(ns,1,group,0,7,43,0,50);
            sc[i]=source.SQUALL_Sample_PlayEx(ss,1,group,0,7,43,0,50);
            same(sc[i],nc[i],"Prepared group channel");require(nc[i]>=0,"Real group channel preparation failed");
            same(source.SQUALL_Channel_Status(sc[i]),native.SQUALL_Channel_Status(nc[i]),"Initial group status");
            require(native.SQUALL_Channel_Status(nc[i])==3,"Prepared channel state differs");
        }
        for(int pause:{0,1}){
            same(source.SQUALL_ChannelGroup_Pause(17,pause),native.SQUALL_ChannelGroup_Pause(17,pause),"Prepared group pause no-op");
            for(unsigned i=0;i<3;++i){
                same(source.SQUALL_Channel_Status(sc[i]),native.SQUALL_Channel_Status(nc[i]),"Prepared group remains prepared");
                require(native.SQUALL_Channel_Status(nc[i])==3,"Pause started a prepared buffer");
            }
        }
        const int otherVolume=native.SQUALL_Channel_GetVolume(nc[2]),otherFrequency=native.SQUALL_Channel_GetFrequency(nc[2]);
        for(int volume:{0,1,43,99,100,101,255}){
            same(source.SQUALL_ChannelGroup_SetVolume(17,volume),native.SQUALL_ChannelGroup_SetVolume(17,volume),"Live group volume");
            for(unsigned i=0;i<3;++i)same(source.SQUALL_Channel_GetVolume(sc[i]),native.SQUALL_Channel_GetVolume(nc[i]),"Grouped volume getter");
            require(native.SQUALL_Channel_GetVolume(nc[2])==otherVolume,"Unrelated group volume changed");
        }
        for(int frequency:{-1,0,99,100,22050,44100,88200,100000,100001,(std::numeric_limits<int>::max)()}){
            same(source.SQUALL_ChannelGroup_SetFrequency(17,frequency),native.SQUALL_ChannelGroup_SetFrequency(17,frequency),"Live group frequency");
            for(unsigned i=0;i<3;++i)same(source.SQUALL_Channel_GetFrequency(sc[i]),native.SQUALL_Channel_GetFrequency(nc[i]),"Grouped frequency getter");
            require(native.SQUALL_Channel_GetFrequency(nc[2])==otherFrequency,"Unrelated group frequency changed");
        }
        same(source.SQUALL_ChannelGroup_SetVolume(17,1),native.SQUALL_ChannelGroup_SetVolume(17,1),"Quiet group playback");
        same(source.SQUALL_ChannelGroup_SetVolume(-7,1),native.SQUALL_ChannelGroup_SetVolume(-7,1),"Quiet other group playback");
        same(source.SQUALL_ChannelGroup_SetFrequency(17,44100),native.SQUALL_ChannelGroup_SetFrequency(17,44100),"Playback frequency");
        for(unsigned i=0;i<3;++i){
            same(source.SQUALL_Channel_Start(sc[i]),native.SQUALL_Channel_Start(nc[i]),"Start actual prepared buffer");
            same(source.SQUALL_Channel_Pause(sc[i],1),native.SQUALL_Channel_Pause(nc[i],1),"Pause actual started buffer");
            require(native.SQUALL_Channel_Status(nc[i])==2,"Started channel did not pause");
        }
        for(int pause:{0,1,-1,0,2,0,1}){
            same(source.SQUALL_ChannelGroup_Pause(17,pause),native.SQUALL_ChannelGroup_Pause(17,pause),"Live group pause");
            for(unsigned i=0;i<3;++i){
                const int wanted=i==2 || pause!=0 ? 2:1;
                same(source.SQUALL_Channel_Status(sc[i]),native.SQUALL_Channel_Status(nc[i]),"Grouped playback status");
                require(native.SQUALL_Channel_Status(nc[i])==wanted,"Group membership or boolean pause semantics differ");
            }
        }
        same(source.SQUALL_ChannelGroup_Pause(-7,0),native.SQUALL_ChannelGroup_Pause(-7,0),"Negative group resume");
        same(source.SQUALL_Channel_Status(sc[2]),native.SQUALL_Channel_Status(nc[2]),"Negative group status");
        require(native.SQUALL_Channel_Status(nc[2])==1 && native.SQUALL_Channel_Status(nc[0])==2,"Negative group membership differs");
        same(source.SQUALL_ChannelGroup_Pause(-7,1),native.SQUALL_ChannelGroup_Pause(-7,1),"Negative group pause");
        auto retire=[&](int group,unsigned first,unsigned end){
            same(source.SQUALL_ChannelGroup_Stop(group),native.SQUALL_ChannelGroup_Stop(group),"Live group stop");
            const auto deadline=GetTickCount64()+2000;
            for(;;){
                bool done=true;
                for(unsigned i=first;i<end;++i){
                    const int a=source.SQUALL_Channel_Status(sc[i]),b=native.SQUALL_Channel_Status(nc[i]);
                    require((a==0 || a==-24) && (b==0 || b==-24),"Unexpected original stop transition");
                    done &= a==0 && b==0;
                }
                if(done)break;
                require(GetTickCount64()<deadline,"Actual group workers did not retire");++retirePolls;Sleep(1);
            }
            for(unsigned i=first;i<end;++i)same(source.SQUALL_Channel_Status(sc[i]),native.SQUALL_Channel_Status(nc[i]),"Retired grouped channel");
        };
        retire(17,0,2);same(source.SQUALL_Channel_Status(sc[2]),native.SQUALL_Channel_Status(nc[2]),"Other group survives stop");require(native.SQUALL_Channel_Status(nc[2])==2,"Other group was stopped");
        retire(-7,2,3);emptyGroups(source,native);
        source.SQUALL_Sample_Unload(ss);native.SQUALL_Sample_Unload(ns);source.SQUALL_Free();native.SQUALL_Free();emptyGroups(source,native);
        require(callbacks.openCount()==0,"Actual callbacks leaked handles");
        require((_control87(0,0)&_MCW_PC)==(argc==5 ? _PC_24:_PC_53),"Precision changed");
        DestroyWindow(window);window=nullptr;
        std::cout<<"{\"export_lookups\":194,\"api_comparisons\":"<<comparisons<<",\"empty_group_calls\":"<<emptyCalls<<",\"prepared_channels\":3,\"group_ids\":[17,-7],\"devices\":"<<devices<<",\"retirement_polls\":"<<retirePolls<<",\"real_directsound\":true,\"original_entry_point_executed\":true,\"audible_output_compared\":false}\n";
        return 0;
    }catch(const std::exception& error){if(window)DestroyWindow(window);std::cerr<<error.what()<<'\n';return 1;}
}
