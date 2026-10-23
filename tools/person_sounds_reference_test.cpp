// Original Person sound bodies are diagnostic only; production PersonSounds
// owns recovered cChannel objects on the source-built Squall DLL.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "PersonSounds.h"
#include "SoundManager.h"
#include "SquallSoundApi.h"
#include "Sound.h"
#include "ResourceManager.h"
#include "FileIndex.h"
#include "Settings.h"
#include "XmlDocument.h"
#include "StringConversion.h"
#include "../src/squall/runtime/Squall.h"
#include <array>
#include <cstring>
#include <filesystem>
#include <float.h>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <vector>
using namespace farmar::recovered;
namespace {
unsigned char* gameImage=nullptr;unsigned phase=0,caseNumber=0,caseKind=0;
LONG CALLBACK diagnose(EXCEPTION_POINTERS* e) {
    if(e->ExceptionRecord->ExceptionCode==EXCEPTION_ACCESS_VIOLATION) {std::cerr<<"Person sound AV phase "<<phase<<" case "<<caseNumber<<" kind "<<caseKind<<" eip "<<std::hex<<e->ContextRecord->Eip<<" gameRVA "<<(e->ContextRecord->Eip-reinterpret_cast<unsigned>(gameImage))<<" address "<<e->ExceptionRecord->ExceptionInformation[1]<<"\n";TerminateProcess(GetCurrentProcess(),1);}return EXCEPTION_CONTINUE_SEARCH;
}
void require(bool ok,const char* message) {if(!ok) throw std::runtime_error(message);}
struct Event {const char* name;std::vector<int> args;int result;};
struct LoadedApi {
    HMODULE module{};
#define SQUALL_ENTRY(name) decltype(&::name) name{};
#include "squall_exports.inc"
#undef SQUALL_ENTRY
    explicit LoadedApi(const std::filesystem::path& path) {
        module=LoadLibraryW(path.c_str());require(module!=nullptr,"LoadLibrary failed");
#define SQUALL_ENTRY(name) name=reinterpret_cast<decltype(name)>(GetProcAddress(module,#name));require(name!=nullptr,#name);
#include "squall_exports.inc"
#undef SQUALL_ENTRY
    }
    ~LoadedApi() {if(module) FreeLibrary(module);}
};
class NativeApi final : public SoundApi {
    LoadedApi& api_;
public:
    explicit NativeApi(LoadedApi& api):api_(api) {}
    int initialize(const SoundSetup& setup) override {squall_parameters_t p;std::memcpy(&p,&setup,sizeof(p));return api_.SQUALL_Init(&p);}
    void free() override {api_.SQUALL_Free();}
    int setFileCallbacks(SoundOpen a,SoundSeek b,SoundRead c,SoundClose d) override {return api_.SQUALL_SetFileCallbacks(a,b,c,d);}
    int setEaxPreset(int value) override {return api_.SQUALL_Listener_EAX_SetPreset(value);}
    int pause(int value) override {return api_.SQUALL_Pause(value);}
    int stop() override {return api_.SQUALL_Stop();}
    int setGroupVolume(int group,int value) override {return api_.SQUALL_ChannelGroup_SetVolume(group,value);}
    int loadSample(const char* name,int memory,const void* defaults) override {return api_.SQUALL_Sample_LoadFile(const_cast<char*>(name),memory,const_cast<squall_sample_default_t*>(static_cast<const squall_sample_default_t*>(defaults)));}
    void unloadSample(int sample) override {api_.SQUALL_Sample_Unload(sample);}
    int sampleFrequency(int sample) override {return api_.SQUALL_Sample_GetFileFrequency(sample);}
    int playSample(int a,int b,int c,int d,int e,int f,int g,int h) override {return api_.SQUALL_Sample_PlayEx(a,b,c,d,e,f,g,h);}
    int stopChannel(int channel) override {return api_.SQUALL_Channel_Stop(channel);}
    int channelStatus(int channel) override {return api_.SQUALL_Channel_Status(channel);}
    int pauseChannel(int channel,int value) override {return api_.SQUALL_Channel_Pause(channel,value);}
    int startChannel(int channel) override {return api_.SQUALL_Channel_Start(channel);}
};
class TraceApi final:public SoundApi {
    SoundApi& backend_;
public:
    explicit TraceApi(SoundApi& backend):backend_(backend) {}
    bool trace=false;std::vector<Event> events;
    int event(const char* name,std::vector<int> args,int value) {if(trace) events.push_back({name,std::move(args),value});return value;}
    int initialize(const SoundSetup& p) override {return backend_.initialize(p);}void free() override {backend_.free();}
    int setFileCallbacks(SoundOpen a,SoundSeek b,SoundRead c,SoundClose d) override {return backend_.setFileCallbacks(a,b,c,d);}
    int setEaxPreset(int x) override {return backend_.setEaxPreset(x);}int pause(int x) override {return backend_.pause(x);}int stop() override {return backend_.stop();}
    int setGroupVolume(int a,int b) override {return backend_.setGroupVolume(a,b);}
    int loadSample(const char* p,int m,const void* d) override {return backend_.loadSample(p,m,d);}void unloadSample(int x) override {backend_.unloadSample(x);}
    int sampleFrequency(int x) override {return event("frequency",{x},backend_.sampleFrequency(x));}
    int playSample(int a,int b,int c,int d,int e,int f,int g,int h) override {return event("play",{a,b,c,d,e,f,g,h},backend_.playSample(a,b,c,d,e,f,g,h));}
    int stopChannel(int x) override {return event("stop",{x},backend_.stopChannel(x));}
    int channelStatus(int x) override {return event("status",{x},backend_.channelStatus(x));}
    int pauseChannel(int x,int p) override {return event("pause",{x,p},backend_.pauseChannel(x,p));}
    int startChannel(int x) override {return event("start",{x},backend_.startChannel(x));}
};
cChannel* __fastcall nativePlay(SoundManager*,void*,cSound* s,bool loop,bool start) {return SoundManager::instance()->playSound(s,loop,start);}
void __fastcall nativeClick(SoundManager*,void*,cSound* s) {SoundManager::instance()->clickSound(s);}
void __fastcall nativeStart(cChannel* p,void*) {p->start();}
void importedCandidateIs(LoadedApi& candidate) {
    // Verify the actual executable IAT, rather than assuming DLL search order.
    auto* base=reinterpret_cast<unsigned char*>(GetModuleHandleW(nullptr));
    auto* pe=reinterpret_cast<IMAGE_NT_HEADERS32*>(base+reinterpret_cast<IMAGE_DOS_HEADER*>(base)->e_lfanew);
    auto* imports=reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(base+pe->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
    unsigned matched=0;
    for(;imports->Name;++imports) {
        if(_stricmp(reinterpret_cast<char*>(base+imports->Name),"Squall.dll")!=0) continue;
        auto* names=reinterpret_cast<IMAGE_THUNK_DATA32*>(base+imports->OriginalFirstThunk);
        auto* addresses=reinterpret_cast<IMAGE_THUNK_DATA32*>(base+imports->FirstThunk);
        for(;names->u1.AddressOfData;++names,++addresses) {
            const char* symbol;
            if(IMAGE_SNAP_BY_ORDINAL32(names->u1.Ordinal)) symbol=MAKEINTRESOURCEA(IMAGE_ORDINAL32(names->u1.Ordinal));
            else symbol=reinterpret_cast<char*>(reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(base+names->u1.AddressOfData)->Name);
            require(addresses->u1.Function==reinterpret_cast<unsigned>(GetProcAddress(candidate.module,symbol)),"Production adapter is not bound to the source candidate DLL");
            ++matched;
        }
    }
    require(matched==15,"Production fifteen-import contract changed");
}
struct Original {
    HMODULE module;unsigned char* image;std::array<void*,3> soundTable{};void* soundObject{};
    explicit Original(const wchar_t* path):module(LoadLibraryExW(path,nullptr,DONT_RESOLVE_DLL_REFERENCES)),image(reinterpret_cast<unsigned char*>(module)) {
        require(module!=nullptr,"Map original game failed");gameImage=image;
        const unsigned fixups[][2]={
#include "person_sounds_oracle_fixups.inc"
        };
        for(const auto& f:fixups) {const unsigned address=reinterpret_cast<unsigned>(image)+f[1]-0x400000;write(f[0],&address,4);}
        soundTable[1]=reinterpret_cast<void*>(&nativePlay);soundTable[2]=reinterpret_cast<void*>(&nativeClick);soundObject=soundTable.data();auto* object=&soundObject;write(0x25f734,&object,4);
        const unsigned jump=reinterpret_cast<unsigned>(&nativeStart)-reinterpret_cast<unsigned>(image+0xfc1d0+5);unsigned char code[5]={0xe9};std::memcpy(code+1,&jump,4);write(0xfc1d0,code,5);
    }
    ~Original() {FreeLibrary(module);}
    void write(unsigned rva,const void* data,unsigned size) {DWORD old,ignored;require(VirtualProtect(image+rva,size,PAGE_EXECUTE_READWRITE,&old)!=0,"Original protection failed");std::memcpy(image+rva,data,size);VirtualProtect(image+rva,size,old,&ignored);}
    void play(void* actor,unsigned kind) {reinterpret_cast<void(__thiscall*)(void*,unsigned)>(image+0x4a820)(actor,kind);}
    void stop(void* actor,unsigned kind) {reinterpret_cast<void(__thiscall*)(void*,unsigned)>(image+0x4ad50)(actor,kind);}
    void stopWork(void* actor,unsigned kind) {const std::map<unsigned,unsigned> functions{{0,0x49860},{1,0x49880},{2,0x49820},{4,0x49840}};reinterpret_cast<void(__thiscall*)(void*)>(image+functions.at(kind))(actor);}
};
template<class T>T& field(void* actor,unsigned offset) {return *reinterpret_cast<T*>(static_cast<unsigned char*>(actor)+offset);}
const std::map<unsigned,unsigned> channelOffsets{{0,0x28b4},{1,0x28b0},{2,0x28a8},{4,0x28b8},{6,0x28ac}};
}
int wmain(int argc,wchar_t** argv) {
    if(argc<3 || argc>5) return 2;
    bool device=false,reference=false;for(int i=3;i<argc;++i) {if(!std::wcscmp(argv[i],L"device")) device=true;else if(!std::wcscmp(argv[i],L"reference-backend")) reference=true;else return 2;}
    try {
        AddVectoredExceptionHandler(1,&diagnose);
        Original original(argv[1]);FileIndex files(toNarrow(WString(argv[2])),String("pack"));files.mountCommon(toNarrow(WString(argv[2])));
        const auto directory=std::filesystem::path(".local")/("person-sounds-runtime-"+std::to_string(GetCurrentProcessId()));std::filesystem::create_directories(directory);files.mountCommon(String(directory.generic_string()));
        Settings settings(files);settings.set(String("Sound"),String("Sound"),0U);settings.set(String("Sound"),String("Music"),0U);
        wchar_t executable[MAX_PATH];require(GetModuleFileNameW(nullptr,executable,MAX_PATH)>0,"Executable path missing");LoadedApi loadedCandidate(std::filesystem::path(executable).parent_path()/L"Squall.dll");importedCandidateIs(loadedCandidate);
        SquallSoundApi candidate;std::unique_ptr<LoadedApi> referenceDLL;std::unique_ptr<NativeApi> nativeAPI;
        if(reference) {referenceDLL=std::make_unique<LoadedApi>(std::filesystem::path(argv[1]).parent_path()/L"Squall.dll");nativeAPI=std::make_unique<NativeApi>(*referenceDLL);}
        // Loading the old DLL runs its CRT startup and may reset the FPU.
        // Select the tested caller context after loading both modules, just
        // as the existing physical game-audio oracle does.
        _control87(device ? _PC_24:_PC_53,_MCW_PC);const auto control=_control87(0,0);
        TraceApi api(reference ? static_cast<SoundApi&>(*nativeAPI):static_cast<SoundApi&>(candidate));unsigned states=0,actions=0,events=0,progressChecks=0,pausedChecks=0;
        {
            SoundManager audio(files,settings,api,0);require(!audio.disabled(),"Source physical sound initialization failed");ResourceManager manager(files);
            XmlDocument person(files,String("Person/Person.xml"));XmlNode* definitions=nullptr;require(person.section(String("Sounds"),definitions),"Actual Person sounds missing");
            const wchar_t* keys[]={L"WorkShovelSound",L"WorkTractorSound",L"WorkWateringCanSound",L"WorkSeedSound",L"WorkScytheSound",L"WorkWellSound",L"WorkHoseSound",L"StuffWarehouseSound"};
            std::array<cResource*,8> sounds{};alignas(4) std::array<unsigned char,0x2b30> actor{};
            for(unsigned i=0;i<8;++i) {const auto path=toNarrow(definitions->attribute(WString(keys[i])));require(!path.empty(),"Actual Person sound path is empty");sounds[i]=manager.getSound(path);field<cResource*>(actor.data(),0x2888+i*4)=manager.getSound(path);}
            const auto pinnedSounds=sounds;auto source=std::make_unique<PersonSounds>(sounds,audio);std::map<int,int> channels;std::vector<int> pendingStops;
            auto* module=reference ? referenceDLL->module:loadedCandidate.module;require(module!=nullptr,"Source Squall module missing");
            const auto getInfo=reinterpret_cast<int(__cdecl*)(squall_channels_t*)>(GetProcAddress(module,"SQUALL_GetChannelsInfo"));
            const auto getPosition=reinterpret_cast<int(__cdecl*)(int)>(GetProcAddress(module,"SQUALL_Channel_GetPlayPosition"));require(getInfo && getPosition,"Physical observers missing");
            const auto references=[&] {std::array<int,8> r{};for(unsigned i=0;i<8;++i) r[i]=pinnedSounds[i]->referenceCount();return r;};
            const auto compareEvents=[&](const std::vector<Event>& a,const std::vector<Event>& b) {
                require(a.size()==b.size(),"Person audio API event count differs");
                for(unsigned i=0;i<a.size();++i) {const auto& x=a[i];const auto& y=b[i];require(!std::strcmp(x.name,y.name) && x.args.size()==y.args.size(),"Person audio API event kind differs");auto args=x.args;
                    if(!std::strcmp(x.name,"play")) {require(x.result>=0 && y.result>=0,"Live Person channel creation failed");channels[x.result]=y.result;}
                    else if(std::strcmp(x.name,"frequency")) {require(channels.count(args[0])!=0,"Person channel identity is not paired");args[0]=channels.at(args[0]);require(x.result==y.result,"Person channel operation return differs");}
                    else require(x.result==y.result,"Person resource frequency differs");
                    require(args==y.args,"Person audio API arguments/order differ");++events;
                }
            };
            const auto check=[&] {
                for(const auto& p:channelOffsets) {auto* a=source ? source->channel(p.first):nullptr;auto* b=field<cChannel*>(actor.data(),p.second);require(bool(a)==bool(b),"Person channel presence differs");if(a) {require(channels.at(a->channelID())==b->channelID() && a->group()==b->group(),"Person channel identity/group differs");const auto deadline=GetTickCount64()+2000;int sa=0,sb=0;for(;;) {sa=api.channelStatus(a->channelID());sb=api.channelStatus(b->channelID());if(sa==1 && sb==1) break;if(GetTickCount64()>=deadline) {std::cerr<<"sound kind "<<p.first<<" action "<<actions<<" channels "<<a->channelID()<<","<<b->channelID()<<" status "<<sa<<","<<sb<<"\n";throw std::runtime_error("Live Person loops did not reach playing state");}Sleep(1);}}}
                ++states;
            };
            const auto settle=[&] {const auto deadline=GetTickCount64()+2000;for(;;) {unsigned owned=0;for(const auto& p:channelOffsets) {if(auto* c=source ? source->channel(p.first):nullptr) owned+=api.channelStatus(c->channelID())==1 || api.channelStatus(c->channelID())==2;if(auto* c=field<cChannel*>(actor.data(),p.second)) owned+=api.channelStatus(c->channelID())==1 || api.channelStatus(c->channelID())==2;}squall_channels_t info{};require(getInfo(&info)>=0,"Live channel counts failed");bool retired=true;for(int id:pendingStops) retired &= api.channelStatus(id)==0;if(retired && unsigned(info.Play+info.Pause+info.Prepare+info.Play3D+info.Pause3D+info.Prepare3D)<=owned) {pendingStops.clear();break;}require(GetTickCount64()<deadline,"Person stopped workers did not retire");Sleep(1);}};
            const auto pair=[&](const std::function<void()>& a,const std::function<void()>& b) {
                api.events.clear();api.trace=true;caseNumber=actions;phase=2;const auto before=references();a();phase=3;const auto middle=references();const auto first=api.events;api.events.clear();b();phase=4;const auto after=references();const auto second=api.events;api.trace=false;
                compareEvents(first,second);for(unsigned i=0;i<8;++i) require(middle[i]-before[i]==after[i]-middle[i],"Person sound reference delta differs");++actions;phase=5;check();
                // One-shot handles belong to the backend, not Person. Stop
                // these diagnostic voices after observing both actual calls.
                for(const auto* list:{&first,&second}) for(const auto& e:*list) {if(!std::strcmp(e.name,"stop") && e.result>=0) pendingStops.push_back(e.args[0]);if(!std::strcmp(e.name,"play") && e.args[1]==0) {api.stopChannel(e.result);pendingStops.push_back(e.result);}}phase=6;settle();
            };
            const auto play=[&](unsigned k) {caseKind=k;pair([&]{source->play(k);},[&]{original.play(actor.data(),k);});};
            const auto stop=[&](unsigned k) {caseKind=k;pair([&]{source->stop(k);},[&]{original.stop(actor.data(),k);});};
            check();for(unsigned k:{0U,1U,2U,4U,6U}) play(k);
            // Five live owned loops per side use actual physical workers.
            for(const auto& p:channelOffsets) for(auto* c:{source->channel(p.first),field<cChannel*>(actor.data(),p.second)}) {const auto initial=getPosition(c->channelID());const auto deadline=GetTickCount64()+2000;int position=initial;while(position==initial && GetTickCount64()<deadline) {Sleep(5);position=getPosition(c->channelID());}require(position>=0 && position!=initial,"Person loop cursor did not advance");++progressChecks;}
            // Cached hose must resume without replacing its owned wrapper,
            // even if an XML reload has cleared its current sound slot.
            for(bool clearSlot:{false,true}) {
                auto* a=source->channel(6);auto* b=field<cChannel*>(actor.data(),0x28ac);a->pause();b->pause();require(api.channelStatus(a->channelID())==2 && api.channelStatus(b->channelID())==2,"Hose pause failed");
                if(!clearSlot) {const auto pa=getPosition(a->channelID()),pb=getPosition(b->channelID());Sleep(40);require(getPosition(a->channelID())==pa && getPosition(b->channelID())==pb,"Paused hose cursor moved");pausedChecks+=2;}
                if(clearSlot) {sounds[6]=nullptr;field<cResource*>(actor.data(),0x2888+6*4)=nullptr;}
                play(6);require(source->channel(6)==a && field<cChannel*>(actor.data(),0x28ac)==b,"Hose resume replaced cached wrapper");
                sounds[6]=pinnedSounds[6];field<cResource*>(actor.data(),0x2888+6*4)=pinnedSounds[6];
            }
            // Active work loops take the newly supplied resource on replay.
            sounds[0]=pinnedSounds[1];field<cResource*>(actor.data(),0x2888)=pinnedSounds[1];play(0);
            sounds[0]=pinnedSounds[0];field<cResource*>(actor.data(),0x2888)=pinnedSounds[0];play(0);
            for(unsigned repeat=0;repeat<4;++repeat) for(unsigned k:{0U,1U,2U,4U,6U,3U,5U,7U,8U,0xffffffffU}) play(k);
            for(unsigned k:{0U,1U,2U,4U,7U,99U,6U,6U}) stop(k);
            for(unsigned k:{6U,6U,0U,2U,1U,4U}) play(k);
            for(unsigned k:{2U,4U,1U,0U,2U,4U,1U,0U}) pair([&]{if(k==0) source->stopShovel();if(k==1) source->stopTractor();if(k==2) source->stopWatering();if(k==4) source->stopScythe();},[&]{original.stopWork(actor.data(),k);});
            stop(6);
            // Optional one-shot/hose resources have actual null guards.
            for(unsigned kind:{3U,5U,6U,7U}) {auto* saved=sounds[kind];sounds[kind]=nullptr;field<cResource*>(actor.data(),0x2888+kind*4)=nullptr;play(kind);sounds[kind]=saved;field<cResource*>(actor.data(),0x2888+kind*4)=saved;}
            for(unsigned k:{0U,1U,2U,4U,6U}) play(k);
            // Exact work-channel order used by the actual Person destructor.
            pair([&]{source.reset();},[&]{original.stop(actor.data(),6);for(unsigned k:{2U,4U,1U,0U}) original.stopWork(actor.data(),k);});
            require(getInfo!=nullptr,"Missing physical count observer");settle();for(const auto& p:channelOffsets) require((!source || !source->channel(p.first)) && !field<cChannel*>(actor.data(),p.second),"Person cleanup retains a channel");
            phase=8;for(unsigned i=0;i<8;++i) {sounds[i]->releaseReference();field<cResource*>(actor.data(),0x2888+i*4)->releaseReference();}manager.clearUseless();require(manager.cachedCount()==0 && audio.files().openCount()==0,"Person resources/callbacks leaked");
        }
        require((_control87(0,0)&_MCW_PC)==(control&_MCW_PC),"Person audio changed x87 precision");
        std::cout<<"{\"states\":"<<states<<",\"actions\":"<<actions<<",\"api_events\":"<<events<<",\"playback_progress_checks\":"<<progressChecks<<",\"paused_cursor_checks\":"<<pausedChecks<<",\"physical_source_dll\":"<<(!reference ? "true":"false")<<",\"audible_output_verified\":false,\"x87_precision\":"<<(control&_MCW_PC)<<"}\n"<<std::flush;return 0;
    }catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}
