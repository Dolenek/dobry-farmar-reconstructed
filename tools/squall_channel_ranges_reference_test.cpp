// Actual public DLL comparison of prepared channel ranges and scalar state.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include "Squall.h"
#include "SoundFiles.h"
#include "StringConversion.h"
#include "SoundEngine/SquallApi.h"
#include <array>
#include <vector>
#include <filesystem>
#include <float.h>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <cstring>
using namespace farmar::recovered;
namespace {
struct Api {
#define SQUALL_ENTRY(name) decltype(&name) name{};
#include "squall_exports.inc"
#undef SQUALL_ENTRY
    HMODULE module{};
    explicit Api(const wchar_t* filename){
        module=LoadLibraryW(std::filesystem::absolute(filename).c_str());if(!module)throw std::runtime_error("LoadLibrary failed");
#define SQUALL_ENTRY(name) name=reinterpret_cast<decltype(name)>(GetProcAddress(module,#name));if(!name)throw std::runtime_error(#name);
#include "squall_exports.inc"
#undef SQUALL_ENTRY
    }
    ~Api(){if(module){SQUALL_Free();FreeLibrary(module);}}
};
unsigned calls=0,outputCases=0,rangeCases=0,seekCases=0,pointerCases=0;int inputA=0,inputB=0;
void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
void same(int a,int b,const char* name){++calls;if(a!=b){std::cerr<<name<<" input="<<inputA<<','<<inputB<<" source="<<a<<" native="<<b<<'\n';throw std::runtime_error("Actual channel API results differ");}}
unsigned sourceSymbol(const Api& source,const std::filesystem::path& dll,const char* name){
    std::ifstream map(dll.parent_path()/"Squall.map");require(bool(map),"Source linker map required");
    auto* base=reinterpret_cast<unsigned char*>(source.module);auto* dos=reinterpret_cast<IMAGE_DOS_HEADER*>(base);auto* pe=reinterpret_cast<IMAGE_NT_HEADERS32*>(base+dos->e_lfanew);
    unsigned preferred=0,timestamp=0;std::string line;
    while(std::getline(map,line)){
        if(line.find("Preferred load address is ")!=std::string::npos)preferred=std::stoul(line.substr(line.find_last_of(' ')+1),nullptr,16);
        if(line.find("Timestamp is ")!=std::string::npos)timestamp=std::stoul(line.substr(line.find("Timestamp is ")+13),nullptr,16);
        if(line.find(name)!=std::string::npos){std::istringstream fields(line);std::string segment,symbol,address;fields>>segment>>symbol>>address;const auto flat=std::stoul(address,nullptr,16);require(preferred && flat>=preferred && flat-preferred<pe->OptionalHeader.SizeOfImage && timestamp==pe->FileHeader.TimeDateStamp,"Map must match loaded source DLL");return unsigned(flat-preferred);}
    }
    throw std::runtime_error(name);
}
struct ProtectedPages {
    unsigned char* data=nullptr;DWORD page=0;
    ProtectedPages(){SYSTEM_INFO info{};GetSystemInfo(&info);page=info.dwPageSize;data=static_cast<unsigned char*>(VirtualAlloc(nullptr,page*3,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));require(data!=nullptr,"Protected output fixture allocation failed");DWORD old;require(VirtualProtect(data+page,page,PAGE_NOACCESS,&old)!=0,"Guard page protection failed");require(VirtualProtect(data+page*2,page,PAGE_READONLY,&old)!=0,"Read-only output protection failed");}
    ~ProtectedPages(){if(data)VirtualFree(data,0,MEM_RELEASE);}
};
void outputs(Api& source,Api& native,int sc,int nc,ProtectedPages& pages){
    for(bool milliseconds:{false,true})for(unsigned mode=0;mode<11;++mode){
        std::array<int,8> a,b;a.fill(0x5a5a5a5a);b=a;
        int *as=a.data()+2,*ae=a.data()+5,*bs=b.data()+2,*be=b.data()+5;
        auto* bad=reinterpret_cast<int*>(1);auto* cross=reinterpret_cast<int*>(pages.data+pages.page-2);auto* readonly=reinterpret_cast<int*>(pages.data+pages.page*2);
        switch(mode){
        case 1:ae=be=nullptr;break;case 2:as=bs=nullptr;break;case 3:as=bs=ae=be=nullptr;break;
        case 4:as=bs=bad;break;case 5:ae=be=bad;break;case 6:as=bs=ae=be=bad;break;
        case 7:ae=as;be=bs;break;case 8:as=bs=cross;break;case 9:as=bs=readonly;break;case 10:ae=be=readonly;break;
        }
        auto own=milliseconds ? source.SQUALL_Channel_GetFragmentMs:source.SQUALL_Channel_GetFragment;
        auto original=milliseconds ? native.SQUALL_Channel_GetFragmentMs:native.SQUALL_Channel_GetFragment;
        same(own(sc,as,ae),original(nc,bs,be),milliseconds ? "GetFragmentMs guarded":"GetFragment guarded");
        require(a==b,"Public fragment output bytes or guards differ");++outputCases;
    }
}
}
int wmain(int argc,wchar_t** argv){
    if(argc<4 || argc>5)return 2;HWND window=nullptr;
    try{
        FileIndex files(toNarrow(WString(argv[3])),String("pack"));files.mountCommon(toNarrow(WString(argv[3])));SoundFiles callbacks(files);
        Api native(argv[1]),source(argv[2]);ProtectedPages pages;_control87(argc==5 ? _PC_24:_PC_53,_MCW_PC);
        const int devices=native.SQUALL_GetNumDevice();same(source.SQUALL_GetNumDevice(),devices,"Device count");require(devices>0,"Actual audio device required");
        window=CreateWindowExW(0,L"STATIC",L"Squall channel ranges comparison",0,0,0,64,64,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);require(window!=nullptr,"Real hidden window required");
        squall_parameters_t parameters{};parameters.Window=window;parameters.SampleRate=44100;parameters.BitPerSample=16;parameters.Channels=16;parameters.UseAlg=3;parameters.BufferSize=200;parameters.DistanceFactor=1;parameters.RolloffFactor=.001f;parameters.DopplerFactor=1;
        const int initialized=native.SQUALL_Init(&parameters);same(source.SQUALL_Init(&parameters),initialized,"Init");require(initialized>=0,"Real DirectSound initialization failed");
        same(source.SQUALL_SetFileCallbacks(&SoundFiles::open,&SoundFiles::seek,&SoundFiles::read,&SoundFiles::close),native.SQUALL_SetFileCallbacks(&SoundFiles::open,&SoundFiles::seek,&SoundFiles::read,&SoundFiles::close),"File callbacks");
        std::string filename="sounds/menu_music.ogg";const int ns=native.SQUALL_Sample_LoadFile(filename.data(),1,nullptr),ss=source.SQUALL_Sample_LoadFile(filename.data(),1,nullptr);same(ss,ns,"Actual sample load");require(ns>0,"Actual sample failed");
        const int nc=native.SQUALL_Sample_PlayEx(ns,0,17,0,7,1,0,50),sc=source.SQUALL_Sample_PlayEx(ss,0,17,0,7,1,0,50);same(sc,nc,"Actual prepared channel");require(nc>=0,"Prepared channel failed");
        const int length=native.SQUALL_Channel_GetLength(nc),duration=native.SQUALL_Channel_GetLengthMs(nc);require(length>13230 && duration>100,"Actual sample is too short");
        same(source.SQUALL_Channel_GetLength(sc),length,"Length");same(source.SQUALL_Channel_GetLengthMs(sc),duration,"LengthMs");outputs(source,native,sc,nc,pages);
        // Equal-address object replay isolates native's pointer-seeded local
        // from the different allocations of the two independently loaded DLLs.
        auto* ownBase=reinterpret_cast<unsigned char*>(source.module);
        auto* context=reinterpret_cast<context_t*>(ownBase+sourceSymbol(source,std::filesystem::absolute(argv[2]),"?api@@3Usquall_api_s@@A")+offsetof(squall_api_t,_context));
        using PositionMethod=int(__thiscall*)(SSample*,int*);
        auto ownPosition=reinterpret_cast<PositionMethod>(ownBase+sourceSymbol(source,std::filesystem::absolute(argv[2]),"?GetPlayPosition@SSample@@"));
        auto originalPosition=reinterpret_cast<PositionMethod>(reinterpret_cast<unsigned char*>(native.module)+0x2180);
        const auto actual=context->_channelsArray[unsigned(sc)&255U];
        std::cerr<<"Equal-address replay\n";
        for(unsigned stage:{0U,1U,2U,3U,4U,5U,6U,7U})for(unsigned buffering:{0U,1U})for(int pause:{0,4,1337})for(int cursor:{0,1,actual.SamplesInBuffer-1,actual.SamplesInBuffer,actual.SamplesInBuffer*2+13}){
            auto fixture=actual;fixture.Status.STAGE=stage;fixture.Status.DOUBLE_BUFFERING=buffering;fixture.PausePtr=pause;fixture.CurPtr=cursor;
            int a=0x5a5a5a5a,b=a;same(ownPosition(&fixture,&a),originalPosition(&fixture,&b),"Equal-address position method");same(a,b,"Equal-address pointer/cursor position");++pointerCases;
        }
        for(const auto& pair:std::vector<std::array<int,2>>{{-1,10},{0,-1},{0,0},{0,1},{1,0},{1,2},{13,13230},{13230,13},{0,length},{length,length+1},{length-1,length+100},{length+1,length+2},{(std::numeric_limits<int>::max)(),(std::numeric_limits<int>::max)()}}){
            std::cerr<<"Frame fragment "<<pair[0]<<','<<pair[1]<<'\n';
            inputA=pair[0];inputB=pair[1];same(source.SQUALL_Channel_SetFragment(sc,inputA,inputB),native.SQUALL_Channel_SetFragment(nc,inputA,inputB),"SetFragment");outputs(source,native,sc,nc,pages);++rangeCases;
        }
        for(const auto& pair:std::vector<std::array<int,2>>{{-1,10},{0,-1},{0,0},{0,1},{1,0},{1,2},{13,300},{300,13},{0,duration},{duration,duration+1},{duration-1,duration+100}}){
            std::cerr<<"Ms fragment "<<pair[0]<<','<<pair[1]<<'\n';
            inputA=pair[0];inputB=pair[1];same(source.SQUALL_Channel_SetFragmentMs(sc,inputA,inputB),native.SQUALL_Channel_SetFragmentMs(nc,inputA,inputB),"SetFragmentMs");outputs(source,native,sc,nc,pages);++rangeCases;
        }
        same(source.SQUALL_Channel_SetFragment(sc,0,length),native.SQUALL_Channel_SetFragment(nc,0,length),"Restore full fragment");
        same(source.SQUALL_Channel_Start(sc),native.SQUALL_Channel_Start(nc),"Start before defined position reads");
        same(source.SQUALL_Channel_Pause(sc,1),native.SQUALL_Channel_Pause(nc,1),"Pause before defined position reads");
        same(source.SQUALL_Channel_SetPlayPosition(sc,0),native.SQUALL_Channel_SetPlayPosition(nc,0),"Zero paused cursor");
        std::vector<int> framePositions{-1,0,1,13,length/2,length-1,length};
        // Opt-in, externally bounded reference experiment: the original can
        // fail to return when a frame position exceeds the end of the file.
        if(GetEnvironmentVariableW(L"FARMAR_RANGE_PAST_END",nullptr,0))framePositions.push_back(length+1);
        const bool sourceFirst=GetEnvironmentVariableW(L"FARMAR_RANGE_SOURCE_FIRST",nullptr,0)!=0;
        for(int value:framePositions){
            std::cerr<<"Frame seek "<<value<<'\n';
            inputA=value;int a,b;
            if(sourceFirst){std::cerr<<"Source frame setter\n";a=source.SQUALL_Channel_SetPlayPosition(sc,value);std::cerr<<"Native frame setter\n";b=native.SQUALL_Channel_SetPlayPosition(nc,value);}
            else{std::cerr<<"Native frame setter\n";b=native.SQUALL_Channel_SetPlayPosition(nc,value);std::cerr<<"Source frame setter\n";a=source.SQUALL_Channel_SetPlayPosition(sc,value);}
            same(a,b,"SetPlayPosition");
            same(source.SQUALL_Channel_GetPlayPosition(sc),native.SQUALL_Channel_GetPlayPosition(nc),"GetPlayPosition paused");same(source.SQUALL_Channel_GetPlayPositionMs(sc),native.SQUALL_Channel_GetPlayPositionMs(nc),"GetPlayPositionMs paused");++seekCases;
        }
        std::vector<int> millisecondPositions{0,1,2,7,duration/2,duration-1,duration};
        if(GetEnvironmentVariableW(L"FARMAR_RANGE_NEGATIVE_MS",nullptr,0))millisecondPositions={-1};
        for(int value:millisecondPositions){
            std::cerr<<"Ms seek "<<value<<'\n';
            inputA=value;int a,b;
            if(sourceFirst){std::cerr<<"Source ms setter\n";a=source.SQUALL_Channel_SetPlayPositionMs(sc,value);std::cerr<<"Native ms setter\n";b=native.SQUALL_Channel_SetPlayPositionMs(nc,value);}
            else{std::cerr<<"Native ms setter\n";b=native.SQUALL_Channel_SetPlayPositionMs(nc,value);std::cerr<<"Source ms setter\n";a=source.SQUALL_Channel_SetPlayPositionMs(nc,value);}
            same(a,b,"SetPlayPositionMs");
            same(source.SQUALL_Channel_GetPlayPosition(sc),native.SQUALL_Channel_GetPlayPosition(nc),"GetPlayPosition prepared");same(source.SQUALL_Channel_GetPlayPositionMs(sc),native.SQUALL_Channel_GetPlayPositionMs(nc),"GetPlayPositionMs prepared");++seekCases;
        }
        same(source.SQUALL_Channel_SetPlayPosition(sc,0),native.SQUALL_Channel_SetPlayPosition(nc,0),"Reset before loop resumes");
        for(int value:{-1,0,1,2,3,255,256,(std::numeric_limits<int>::max)()}){
            std::cerr<<"Loop "<<value<<'\n';
            inputA=value;same(source.SQUALL_Channel_SetLoop(sc,value),native.SQUALL_Channel_SetLoop(nc,value),"SetLoop");same(source.SQUALL_Channel_GetLoop(sc),native.SQUALL_Channel_GetLoop(nc),"GetLoop");same(source.SQUALL_Channel_Status(sc),native.SQUALL_Channel_Status(nc),"SetLoop prepared status");
        }
        for(int value:{-1,0,1,65535,65536,(std::numeric_limits<int>::max)(),(std::numeric_limits<int>::min)()}){
            std::cerr<<"Priority "<<value<<'\n';
            inputA=value;same(source.SQUALL_Channel_SetPriority(sc,value),native.SQUALL_Channel_SetPriority(nc,value),"SetPriority");same(source.SQUALL_Channel_GetPriority(sc),native.SQUALL_Channel_GetPriority(nc),"GetPriority");
        }
        for(int id:{0,-1,(std::numeric_limits<int>::max)(),nc^0x40000000}){
            inputA=id;same(source.SQUALL_Channel_Status(id),native.SQUALL_Channel_Status(id),"Invalid status");require(native.SQUALL_Channel_Status(id)==0,"Diagnostic invalid ID is live");outputs(source,native,id,id,pages);
            same(source.SQUALL_Channel_SetFragment(id,0,1),native.SQUALL_Channel_SetFragment(id,0,1),"Invalid SetFragment");same(source.SQUALL_Channel_SetFragmentMs(id,0,1),native.SQUALL_Channel_SetFragmentMs(id,0,1),"Invalid SetFragmentMs");
            same(source.SQUALL_Channel_SetLoop(id,1),native.SQUALL_Channel_SetLoop(id,1),"Invalid SetLoop");same(source.SQUALL_Channel_GetLoop(id),native.SQUALL_Channel_GetLoop(id),"Invalid GetLoop");
            same(source.SQUALL_Channel_SetPriority(id,1),native.SQUALL_Channel_SetPriority(id,1),"Invalid SetPriority");same(source.SQUALL_Channel_GetPriority(id),native.SQUALL_Channel_GetPriority(id),"Invalid GetPriority");
            same(source.SQUALL_Channel_GetPlayPosition(id),native.SQUALL_Channel_GetPlayPosition(id),"Invalid GetPlayPosition");same(source.SQUALL_Channel_GetPlayPositionMs(id),native.SQUALL_Channel_GetPlayPositionMs(id),"Invalid GetPlayPositionMs");
        }
        same(source.SQUALL_Channel_Stop(sc),native.SQUALL_Channel_Stop(nc),"Stop prepared channel");const auto deadline=GetTickCount64()+2000;
        for(;;){const int a=source.SQUALL_Channel_Status(sc),b=native.SQUALL_Channel_Status(nc);require((a==0 || a==-24)&&(b==0 || b==-24),"Unexpected stop state");if(!a&&!b)break;require(GetTickCount64()<deadline,"Workers did not retire");Sleep(1);}
        source.SQUALL_Sample_Unload(ss);native.SQUALL_Sample_Unload(ns);source.SQUALL_Free();native.SQUALL_Free();require(callbacks.openCount()==0,"Callback handles leaked");require((_control87(0,0)&_MCW_PC)==(argc==5 ? _PC_24:_PC_53),"Precision changed");DestroyWindow(window);window=nullptr;
        std::cout<<"{\"export_lookups\":194,\"api_comparisons\":"<<calls<<",\"output_pointer_cases\":"<<outputCases<<",\"range_setter_cases\":"<<rangeCases<<",\"seek_cases\":"<<seekCases<<",\"equal_address_position_cases\":"<<pointerCases<<",\"prepared_channels\":1,\"real_directsound\":true,\"audible_output_compared\":false}\n";return 0;
    }catch(const std::exception& error){if(window)DestroyWindow(window);std::cerr<<error.what()<<'\n';return 1;}
}
