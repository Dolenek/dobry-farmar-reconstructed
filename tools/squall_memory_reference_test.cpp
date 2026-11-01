#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "Squall.h"
#include <array>
#include <vector>
#include <map>
#include <filesystem>
#include <float.h>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
namespace {
void need(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
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
    ~Api(){if(module){SQUALL_Sample_UnloadAll();SQUALL_Free();FreeLibrary(module);}}
};
struct Event {
    unsigned operation,generation,size,serial;
    bool operator==(const Event& b)const{return operation==b.operation&&generation==b.generation&&size==b.size&&serial==b.serial;}
};
struct Allocation {unsigned size,serial,generation;unsigned char* base;};
struct Tracker {
    std::map<void*,Allocation> live;
    std::vector<Event> events;
    unsigned calls{},serial{},failAt{};
    bool invalidFree{},guardFailure{};
} trackers[2];
template<unsigned D,unsigned G> void* __cdecl allocate(unsigned size){
    auto& t=trackers[D];++t.calls;
    if(t.failAt==t.calls){t.events.push_back({2,G,size,0});return nullptr;}
    auto* base=static_cast<unsigned char*>(HeapAlloc(GetProcessHeap(),0,size+32));
    if(!base){t.events.push_back({2,G,size,0});return nullptr;}
    std::memset(base,0xa7,16);std::memset(base+16+size,0x7a,16);
    auto* p=base+16;const auto id=++t.serial;t.live.emplace(p,Allocation{size,id,G,base});t.events.push_back({0,G,size,id});return p;
}
template<unsigned D,unsigned G> void __cdecl release(void* p){
    auto& t=trackers[D];auto it=t.live.find(p);
    if(it==t.live.end()){t.invalidFree=true;return;}
    auto a=it->second;t.invalidFree|=a.generation!=G;
    for(unsigned i=0;i<16;++i)t.guardFailure|=a.base[i]!=0xa7||a.base[16+a.size+i]!=0x7a;
    t.events.push_back({1,G,a.size,a.serial});HeapFree(GetProcessHeap(),0,a.base);t.live.erase(it);
}
unsigned results{},snapshots{},callbackSnapshots{},phases{},loaded{},failedLoads{},validityCases{};
void same(int a,int b,const char* name){++results;if(a!=b){std::cerr<<name<<" source="<<a<<" native="<<b<<'\n';throw std::runtime_error("Memory callback result differs");}}
void callbacks(){
    const auto& a=trackers[0];const auto& b=trackers[1];
    need(!a.invalidFree&&!b.invalidFree,"Incorrect callback owns allocation");need(!a.guardFailure&&!b.guardFailure,"Callback allocation guard changed");
    if(a.events!=b.events){std::cerr<<"Callback events native="<<a.events.size()<<" source="<<b.events.size()<<'\n';throw std::runtime_error("Callback event order/size/owner differs");}
    need(a.live.size()==b.live.size(),"Live callback allocations differ");++callbackSnapshots;
}
void set(Api& s,Api& n,unsigned generation){
    if(generation==0)same(s.SQUALL_SetMemoryCallbacks(allocate<1,0>,release<1,0>),n.SQUALL_SetMemoryCallbacks(allocate<0,0>,release<0,0>),"Set callbacks A");
    else same(s.SQUALL_SetMemoryCallbacks(allocate<1,1>,release<1,1>),n.SQUALL_SetMemoryCallbacks(allocate<0,1>,release<0,1>),"Set callbacks B");
    callbacks();
}
void inspect(Api& s,Api& n,int own,int native){
    same(s.SQUALL_Sample_GetFileLength(own),n.SQUALL_Sample_GetFileLength(native),"Sample frames");
    same(s.SQUALL_Sample_GetFileLengthMs(own),n.SQUALL_Sample_GetFileLengthMs(native),"Sample milliseconds");
    same(s.SQUALL_Sample_GetFileFrequency(own),n.SQUALL_Sample_GetFileFrequency(native),"Sample frequency");
    std::array<std::uint32_t,11>a,b;a.fill(0x5a5a5a5a);b=a;
    same(s.SQUALL_Sample_GetDefault(own,reinterpret_cast<squall_sample_default_t*>(a.data()+2)),n.SQUALL_Sample_GetDefault(native,reinterpret_cast<squall_sample_default_t*>(b.data()+2)),"Guarded sample defaults");
    need(a==b,"Sample default/guard differs");++snapshots;callbacks();
}
std::array<int,2> load(Api& s,Api& n,const char* file,int memory,bool success=true){
    const int b=n.SQUALL_Sample_LoadFile(const_cast<char*>(file),memory,nullptr),a=s.SQUALL_Sample_LoadFile(const_cast<char*>(file),memory,nullptr);
    same(a,b,"Load real WAV file");if(success){need(a>0,"Valid WAV sample failed");++loaded;inspect(s,n,a,b);need(n.SQUALL_Sample_GetFileLength(b)==64&&n.SQUALL_Sample_GetFileLengthMs(b)==8&&n.SQUALL_Sample_GetFileFrequency(b)==8000,"Unexpected WAV format/length");}else{need(a<0,"Forced allocation failure not reported");++failedLoads;callbacks();}
    return {a,b};
}
void unload(Api& s,Api& n,std::array<int,2> id){s.SQUALL_Sample_Unload(id[0]);n.SQUALL_Sample_Unload(id[1]);callbacks();inspect(s,n,id[0],id[1]);}
void unloadAll(Api& s,Api& n){s.SQUALL_Sample_UnloadAll();n.SQUALL_Sample_UnloadAll();callbacks();need(trackers[0].live.empty()&&trackers[1].live.empty(),"UnloadAll leaked reader allocations");}
void validity(Api& s,Api& n){
    auto* page=VirtualAlloc(nullptr,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);need(page!=nullptr,"Allocate pointer page");
    for(unsigned i=0;i<6;++i)for(unsigned j=0;j<6;++j){
        auto alloc=[&](bool source,unsigned index)->squall_callback_alloc_t{
            if(index==0)return source?allocate<1,0>:allocate<0,0>;
            if(index==1)return source?allocate<1,1>:allocate<0,1>;
            if(index==2)return nullptr;if(index==3)return reinterpret_cast<squall_callback_alloc_t>(1);
            if(index==4)return reinterpret_cast<squall_callback_alloc_t>(~std::uintptr_t(0));return reinterpret_cast<squall_callback_alloc_t>(page);
        };
        auto free=[&](bool source,unsigned index)->squall_callback_free_t{
            if(index==0)return source?release<1,0>:release<0,0>;
            if(index==1)return source?release<1,1>:release<0,1>;
            if(index==2)return nullptr;if(index==3)return reinterpret_cast<squall_callback_free_t>(1);
            if(index==4)return reinterpret_cast<squall_callback_free_t>(~std::uintptr_t(0));return reinterpret_cast<squall_callback_free_t>(page);
        };
        same(s.SQUALL_SetMemoryCallbacks(alloc(true,i),free(true,j)),n.SQUALL_SetMemoryCallbacks(alloc(false,i),free(false,j)),"Callback pointer pair");++validityCases;
        // The original stores invalid pointers even when returning an error.
        // Reset both before any operation can invoke them.
        set(s,n,0);
    }
    DWORD previous{};need(VirtualProtect(page,4096,PAGE_NOACCESS,&previous)!=FALSE,"Guard callback page");
    same(s.SQUALL_SetMemoryCallbacks(reinterpret_cast<squall_callback_alloc_t>(page),release<1,0>),n.SQUALL_SetMemoryCallbacks(reinterpret_cast<squall_callback_alloc_t>(page),release<0,0>),"No-access allocator");set(s,n,0);
    same(s.SQUALL_SetMemoryCallbacks(allocate<1,0>,reinterpret_cast<squall_callback_free_t>(page)),n.SQUALL_SetMemoryCallbacks(allocate<0,0>,reinterpret_cast<squall_callback_free_t>(page)),"No-access free");set(s,n,0);validityCases+=2;
    VirtualFree(page,0,MEM_RELEASE);
}
void cases(Api& s,Api& n){
    ++phases;validity(s,n);
    for(int memory:{0,1,-1,2}){
        set(s,n,0);const auto a=load(s,n,"sample.wav",memory);
        set(s,n,1);const auto b=load(s,n,"sample2.wav",memory);
        // Duplicate file has its own sample ID/defaults, sharing the A-owned reader.
        const auto duplicate=load(s,n,"sample.wav",memory);unload(s,n,a);inspect(s,n,duplicate[0],duplicate[1]);unload(s,n,b);unload(s,n,duplicate);
        need(trackers[0].live.empty(),"Individual unload leaked reader allocations");
    }
    for(int memory:{0,1})for(unsigned fail:{1U,2U}){
        if(memory==0&&fail==2)continue;set(s,n,0);
        for(auto& t:trackers)t.failAt=t.calls+fail;
        load(s,n,"sample.wav",memory,false);
        for(auto& t:trackers)t.failAt=0;
        callbacks();need(trackers[0].live.empty(),"Failed reader initialization leaked allocation");
        const auto recovered=load(s,n,"sample.wav",memory);unload(s,n,recovered);
    }
    set(s,n,0);auto a=load(s,n,"sample.wav",1);set(s,n,1);auto b=load(s,n,"sample2.wav",0);
    unloadAll(s,n);inspect(s,n,a[0],a[1]);inspect(s,n,b[0],b[1]);unloadAll(s,n);
}
}
int wmain(int argc,wchar_t** argv){
    if(argc<3||argc>4)return 2;HWND window{};
    try{
        Api native(argv[1]),source(argv[2]);const auto precision=argc==4?_PC_24:_PC_53;_control87(precision,_MCW_PC);
        cases(source,native);
        window=CreateWindowExW(0,L"STATIC",L"Squall memory callbacks",0,0,0,64,64,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);need(window!=nullptr,"Create DirectSound window");
        squall_parameters_t p{};p.Window=window;p.SampleRate=44100;p.BitPerSample=16;p.Channels=4;p.UseAlg=3;p.BufferSize=200;p.DistanceFactor=1;p.RolloffFactor=.001f;p.DopplerFactor=1;
        auto initialized=native.SQUALL_Init(&p);same(source.SQUALL_Init(&p),initialized,"Actual DirectSound Init");need(initialized>=0,"DirectSound initialization failed");
        cases(source,native);source.SQUALL_Free();native.SQUALL_Free();cases(source,native);
        need((_control87(0,0)&_MCW_PC)==precision,"Precision changed");DestroyWindow(window);window=nullptr;
        std::cout<<"{\"result_comparisons\":"<<results<<",\"guarded_sample_snapshots\":"<<snapshots<<",\"callback_event_snapshots\":"<<callbackSnapshots<<",\"callback_events_per_dll\":"<<trackers[0].events.size()<<",\"pointer_pairs\":"<<validityCases<<",\"loaded_samples_per_dll\":"<<loaded<<",\"forced_failed_loads_per_dll\":"<<failedLoads<<",\"lifecycle_phases\":"<<phases<<",\"export_lookups\":194,\"actual_directsound\":true,\"callback_events\":[";
        for(unsigned i=0;i<trackers[0].events.size();++i){auto e=trackers[0].events[i];if(i)std::cout<<',';std::cout<<'['<<e.operation<<','<<e.generation<<','<<e.size<<','<<e.serial<<']';}
        std::cout<<"]}\n";return 0;
    }catch(const std::exception& error){if(window)DestroyWindow(window);std::cerr<<error.what()<<'\n';return 1;}
}
