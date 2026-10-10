// Native routines are test oracles only; no original entry point or Squall
// implementation runs. FileIndex/iFile and Squall API boundaries are explicit.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "SoundFiles.h"
#include "SoundVolume.h"
#include "SoundManager.h"
#include "Sound.h"
#include "Channel.h"
#include "Settings.h"
#include "XmlDocument.h"
#include "ResourceManager.h"
#include "UiSound.h"
#include "StringConversion.h"
#include <array>
#include <cstdlib>
#include <cstring>
#include <float.h>
#include <iostream>
#include <random>
#include <set>
#include <stdexcept>
#include <vector>
using namespace farmar::recovered;
namespace {
unsigned char* image=nullptr;unsigned trial=0,step=0;
template<class T> T& field(void* p,unsigned offset) { return *reinterpret_cast<T*>(static_cast<unsigned char*>(p)+offset); }
template<class T> T function(unsigned rva) { return reinterpret_cast<T>(image+rva); }
void require(bool result,const char* error) { if(!result) throw std::runtime_error(error); }
void pointer(unsigned rva,void* value) { std::memcpy(image+rva,&value,4); }
void redirect(unsigned rva,void* value) { unsigned char bytes[5]={0xe9};const unsigned offset=reinterpret_cast<unsigned>(value)-reinterpret_cast<unsigned>(image+rva+5);std::memcpy(bytes+1,&offset,4);std::memcpy(image+rva,bytes,5); }
LONG CALLBACK diagnose(EXCEPTION_POINTERS* e) {
    if(e->ExceptionRecord->ExceptionCode==EXCEPTION_ACCESS_VIOLATION) {
        std::cerr<<"Sound oracle AV trial "<<trial<<" step "<<step<<" RVA "<<std::hex<<reinterpret_cast<unsigned>(e->ExceptionRecord->ExceptionAddress)-reinterpret_cast<unsigned>(image)<<" target "<<e->ExceptionRecord->ExceptionInformation[1]<<'\n';TerminateProcess(GetCurrentProcess(),1);
    }
    return EXCEPTION_CONTINUE_SEARCH;
}
std::set<void*> allocations;
void* __cdecl allocate(unsigned size) { auto* p=std::malloc(size);if(p) allocations.insert(p);return p; }
void __cdecl release(void* p) { allocations.erase(p);std::free(p); }
String* __fastcall assign(String* self,void*,const char* bytes,unsigned size) { return &self->assign(bytes,size); }
String* __fastcall copyString(String* self,void*,const String* value,unsigned first,unsigned size) { return &self->assign(*value,first,size); }
struct FilesAdapter { void** table;FileIndex* files; };
iFile* __fastcall openFile(FilesAdapter* self,void*,const String* name) {
    if(name->view()=="#oracle-null#") return nullptr;
    return self->files->open(*name).release();
}
unsigned initializations=0,frees=0,groupCalls=0;unsigned group=0,volume=0;
struct AudioEvent {
    int kind;std::vector<long long> values;std::string text;
    bool operator==(const AudioEvent& other) const { return kind==other.kind && values==other.values && text==other.text; }
};
struct ApiFixture;
ApiFixture* activeApi=nullptr;
struct ApiFixture final:SoundApi {
    std::vector<AudioEvent> events;int initResult=0,nextSample=100,nextChannel=200;
    std::vector<int> statuses{1};unsigned statusIndex=0;unsigned openHandle=0;
    SoundOpen openCallback=nullptr;SoundSeek seekCallback=nullptr;SoundRead readCallback=nullptr;SoundClose closeCallback=nullptr;
    void record(int kind,std::initializer_list<long long> values={},std::string text={}) { events.push_back({kind,values,std::move(text)}); }
    int initialize(const SoundSetup&) override;
    void free() override;
    int setFileCallbacks(SoundOpen,SoundSeek,SoundRead,SoundClose) override;
    int setEaxPreset(int) override;
    int pause(int) override;
    int stop() override;
    int setGroupVolume(int,int) override;
    int loadSample(const char*,int,const void*) override;
    int unloadSample(int) override;
    int sampleFrequency(int) override;
    int playSample(int,int,int,int,int,int,int,int) override;
    int stopChannel(int) override;
    int channelStatus(int) override;
    int pauseChannel(int,int) override;
    int startChannel(int) override;
};
int __cdecl initialize(const unsigned* setup) {
    const unsigned expected[]={0,0,44100,16,16,0,0,3,200,0,0x3f800000,0x3a83126f,0x3f800000};
    require(!std::memcmp(setup,expected,sizeof(expected)),"Sound initialization parameters differ");++initializations;
    if(activeApi) { activeApi->record(0);return activeApi->initResult; } return -1;
}
void __cdecl freeSound() { ++frees;if(activeApi) { activeApi->record(1);if(activeApi->openHandle) { activeApi->closeCallback(activeApi->openHandle);activeApi->openHandle=0; } } }
int __cdecl groupVolume(unsigned nextGroup,unsigned nextVolume) { group=nextGroup;volume=nextVolume;++groupCalls;if(activeApi) activeApi->record(6,{nextGroup,static_cast<int>(nextVolume)});return -73; }
int __cdecl setCallbacks(SoundOpen open,SoundSeek seek,SoundRead read,SoundClose close) {
    require(activeApi && open && seek && read && close,"Sound callbacks are missing");
    activeApi->openCallback=open;activeApi->seekCallback=seek;activeApi->readCallback=read;activeApi->closeCallback=close;activeApi->record(2);return 0;
}
int __cdecl eaxPreset(int value) { activeApi->record(3,{value});return -17; }
int __cdecl pauseAll(int value) { activeApi->record(4,{value});return -3; }
int __cdecl stopAll() { activeApi->record(5);return -3; }
int __cdecl loadSample(const char* name,int memory,int spatial) { activeApi->record(7,{memory,spatial},name);return activeApi->nextSample++; }
int __cdecl unloadSample(int value) { activeApi->record(8,{value});return -3; }
int __cdecl sampleFrequency(int value) { activeApi->record(9,{value});return 22050+value; }
int __cdecl playSample(int sample,int loop,int nextGroup,int start,int priority,int nextVolume,int frequency,int pan) { activeApi->record(10,{sample,loop,nextGroup,start,priority,nextVolume,frequency,pan});return activeApi->nextChannel++; }
int __cdecl stopChannel(int value) { activeApi->record(11,{value});return -3; }
int __cdecl channelStatus(int value) { const auto result=activeApi->statuses[std::min(activeApi->statusIndex++,unsigned(activeApi->statuses.size()-1))];activeApi->record(12,{value,result});return result; }
int __cdecl pauseChannel(int value,int pause) { activeApi->record(13,{value,pause});return -3; }
int __cdecl startChannel(int value) { activeApi->record(14,{value});return -3; }
int ApiFixture::initialize(const SoundSetup& setup) { return ::initialize(reinterpret_cast<const unsigned*>(&setup)); }
void ApiFixture::free() { freeSound(); }
int ApiFixture::setFileCallbacks(SoundOpen a,SoundSeek b,SoundRead c,SoundClose d) { return setCallbacks(a,b,c,d); }
int ApiFixture::setEaxPreset(int a) { return eaxPreset(a); }
int ApiFixture::pause(int a) { return pauseAll(a); }
int ApiFixture::stop() { return stopAll(); }
int ApiFixture::setGroupVolume(int a,int b) { return groupVolume(a,b); }
int ApiFixture::loadSample(const char* a,int b,const void* defaults) { return ::loadSample(a,b,static_cast<int>(reinterpret_cast<unsigned>(defaults))); }
int ApiFixture::unloadSample(int a) { return ::unloadSample(a); }
int ApiFixture::sampleFrequency(int a) { return ::sampleFrequency(a); }
int ApiFixture::playSample(int a,int b,int c,int d,int e,int f,int g,int h) { return ::playSample(a,b,c,d,e,f,g,h); }
int ApiFixture::stopChannel(int a) { return ::stopChannel(a); }
int ApiFixture::channelStatus(int a) { return ::channelStatus(a); }
int ApiFixture::pauseChannel(int a,int b) { return ::pauseChannel(a,b); }
int ApiFixture::startChannel(int a) { return ::startChannel(a); }
struct SettingsAdapter { void** table;Settings* settings; };
bool __fastcall getUnsigned(SettingsAdapter* self,void*,const String* section,const String* key,unsigned* output) { return self->settings->get(*section,*key,*output); }
void __fastcall setUnsigned(SettingsAdapter* self,void*,const String* section,const String* key,const unsigned* value) { self->settings->set(*section,*key,*value); }
bool sameSettings(const Settings& a,const Settings& b) {
    if(a.sections().size()!=b.sections().size()) return false;
    for(const auto& section:a.sections()) {
        auto match=b.sections().find(section.first);if(match==b.sections().end() || section.second.size()!=match->second.size()) return false;
        for(const auto& property:section.second) { auto item=match->second.find(property.first);if(item==match->second.end() || property.second.view()!=item->second.view()) return false; }
    }
    return true;
}
}
int wmain(int argc,wchar_t** argv) {
    if(argc<3 || argc>5) return 2;
    const bool managerMode=argc>=4 && std::wcscmp(argv[3],L"manager")==0;
    auto reference=LoadLibraryExW(argv[1],nullptr,DONT_RESOLVE_DLL_REFERENCES);if(!reference) return 2;
    try {
        if((argc==4 && !managerMode) || argc==5) _control87(_PC_24,_MCW_PC);AddVectoredExceptionHandler(1,&diagnose);
        auto* loaded=reinterpret_cast<unsigned char*>(reference);
        auto* headers=reinterpret_cast<IMAGE_NT_HEADERS*>(loaded+reinterpret_cast<IMAGE_DOS_HEADER*>(loaded)->e_lfanew);
        image=static_cast<unsigned char*>(VirtualAlloc(nullptr,headers->OptionalHeader.SizeOfImage,MEM_RESERVE|MEM_COMMIT,PAGE_EXECUTE_READWRITE));
        require(image && image!=loaded,"Sound oracle relocation failed");std::memcpy(image,loaded,headers->OptionalHeader.SizeOfImage);
        struct Fixup { unsigned rva,address; };const Fixup fixups[]={
#include "sound_manager_oracle_fixups.inc"
        };
        for(const auto& fixup:fixups) { require(field<unsigned>(image,fixup.rva)==fixup.address,"Sound oracle operand differs");const unsigned value=reinterpret_cast<unsigned>(image)+fixup.address-0x400000;std::memcpy(image+fixup.rva,&value,4); }
        redirect(0x17edb1,reinterpret_cast<void*>(&allocate));redirect(0x1803af,reinterpret_cast<void*>(&release));
        redirect(0x19c0,reinterpret_cast<void*>(&assign));redirect(0x1573b2,reinterpret_cast<void*>(&initialize));
        redirect(0x16d0,reinterpret_cast<void*>(&copyString));
        redirect(0x15739a,reinterpret_cast<void*>(&freeSound));redirect(0x15737c,reinterpret_cast<void*>(&groupVolume));
        redirect(0x1573ac,reinterpret_cast<void*>(&setCallbacks));redirect(0x1573a6,reinterpret_cast<void*>(&eaxPreset));
        redirect(0x157376,reinterpret_cast<void*>(&pauseAll));redirect(0x1573a0,reinterpret_cast<void*>(&stopAll));
        redirect(0x157382,reinterpret_cast<void*>(&loadSample));redirect(0x157388,reinterpret_cast<void*>(&unloadSample));
        redirect(0x157394,reinterpret_cast<void*>(&sampleFrequency));redirect(0x15738e,reinterpret_cast<void*>(&playSample));
        redirect(0x11d87a,reinterpret_cast<void*>(&stopChannel));redirect(0x11d880,reinterpret_cast<void*>(&channelStatus));
        redirect(0x11d886,reinterpret_cast<void*>(&pauseChannel));redirect(0x11d88c,reinterpret_cast<void*>(&startChannel));
        FlushInstructionCache(GetCurrentProcess(),image,headers->OptionalHeader.SizeOfImage);
        FileIndex files(toNarrow(WString(argv[2])),String("pack"));files.mountCommon(toNarrow(WString(argv[2])));
        std::array<void*,3> fileTable{};fileTable[2]=reinterpret_cast<void*>(&openFile);FilesAdapter fileAdapter{fileTable.data(),&files};pointer(0x25f72c,&fileAdapter);
        std::vector<String> names;
        for(const auto& item:files.files()) if(item.first.size()>4 && (item.first.substr(item.first.size()-4)==".wav" || item.first.substr(item.first.size()-4)==".ogg")) names.emplace_back(item.first.c_str());
        require(names.size()>20,"Sound data indexing fixture is incomplete");
        const auto nativeOpen=function<unsigned(__cdecl*)(const char*)>(0x135900);
        const auto nativeSeek=function<int(__cdecl*)(unsigned,int,int)>(0x133ea0);
        const auto nativeRead=function<int(__cdecl*)(unsigned,void*,int)>(0x133a90);
        const auto nativeClose=function<void(__cdecl*)(unsigned)>(0x1342c0);
        unsigned opens=0,missing=0,reads=0,seeks=0,bytes=0,volumeChecks=0;std::mt19937 random(0x534f554e);
        for(trial=0;trial<32;++trial) {
            SoundFiles actual(files);std::array<unsigned char,40> native{};
            function<void*(__thiscall*)(void*)>(0x135d50)(native.data());
            require(field<unsigned char>(native.data(),0x24)==1 && field<unsigned>(native.data(),0x10)==0,"Native disabled initialization differs");
            require(field<unsigned>(image,0x25d384)==opens+1,"Persistent sound handle counter differs");
            // Native null-return branch is an injected file-manager outcome.
            // Actual FileIndex throws for absent names; do not unwind a modern
            // exception through the old, unmapped native CRT exception frames.
            require(nativeOpen("#oracle-null#")==0xffffffffU,"Native null file return differs");
            bool failed=false;try { SoundFiles::open("missing-sound-fixture.wav"); } catch(const std::runtime_error&) { failed=true; }
            require(failed,"Missing source FileIndex exception was lost");++missing;
            std::vector<unsigned> handles;
            for(step=0;step<12;++step) {
                const auto& name=names[(trial*17+step*7)%names.size()];
                const unsigned handle=SoundFiles::open(name.c_str()),expected=nativeOpen(name.c_str());++opens;
                require(handle==expected && handle==opens+1,"Sound handle allocation differs");handles.push_back(handle);
                require(actual.openCount()==handles.size() && field<unsigned>(native.data(),0x10)==handles.size(),"Sound handle map size differs");
                for(unsigned i=0;i<8;++i) {
                    const int position=i%2 ? 0:13,mode=int(i%4);
                    require(SoundFiles::seek(handle,position,mode)==nativeSeek(handle,position,mode),"Sound size-only seek differs");++seeks;
                    std::array<unsigned char,4096> actualBytes{},nativeBytes{};
                    const int count=i==0 ? 0:int(random()%actualBytes.size());
                    const int actualRead=SoundFiles::read(handle,actualBytes.data(),count),expectedRead=nativeRead(handle,nativeBytes.data(),count);
                    require(actualRead==expectedRead && !std::memcmp(actualBytes.data(),nativeBytes.data(),actualBytes.size()),"Sound file read bytes/position differ");++reads;bytes+=unsigned(actualRead);
                }
            }
            // Closing in randomized order exercises both branches of tree erase.
            while(!handles.empty()) { const auto index=random()%handles.size(),handle=handles[index];SoundFiles::close(handle);nativeClose(handle);handles.erase(handles.begin()+index);require(actual.openCount()==handles.size() && field<unsigned>(native.data(),0x10)==handles.size(),"Sound close map state differs"); }
            function<void(__thiscall*)(void*)>(0x135280)(native.data());
            require(!field<void*>(image,0x25f734) && allocations.empty(),"Sound file map/singleton cleanup differs");
        }
        // Preserve original ftol2 and x87 approximation instructions as oracle;
        // production performs editable arithmetic, not calls into the reference.
        std::array<unsigned char,40> nativeVolume{};
        for(step=0;step<100101;++step) {
            const unsigned value=step<101 ? step:step%13==0 ? 0xffffffffU:step%17==0 ? 0x80000000U:random();
            const unsigned transformed=soundGroupVolume(value);field<unsigned char>(nativeVolume.data(),0x24)=static_cast<unsigned char>(step%2);
            const unsigned before=groupCalls;
            function<void(__thiscall*)(void*,unsigned)>(0x132f40)(nativeVolume.data(),value);
            require(field<unsigned>(nativeVolume.data(),0x18)==value && field<unsigned>(nativeVolume.data(),0x20)==transformed,"Sound volume approximation differs");++volumeChecks;
            if(step%2==0) require(groupCalls==before+1 && group==1 && volume==transformed,"Sound group API arguments differ");else require(groupCalls==before,"Disabled sound requested group volume");
            function<void(__thiscall*)(void*,unsigned)>(0x132f90)(nativeVolume.data(),value);
            require(field<unsigned>(nativeVolume.data(),0x14)==value && field<unsigned>(nativeVolume.data(),0x1c)==transformed,"Music volume approximation differs");++volumeChecks;
            if(step%2==0) require(groupCalls==before+2 && group==2 && volume==transformed,"Music group API arguments differ");else require(groupCalls==before,"Disabled music requested group volume");
        }
        unsigned managerStates=0,managerLifetimes=0,sampleObjects=0,channelObjects=0,apiEvents=0;
        if(managerMode) {
#include "sound_manager_api_cases.inc"
        }
        std::cout<<"{\"lifetimes\":32,\"opens\":"<<opens<<",\"missing\":"<<missing<<",\"reads\":"<<reads<<",\"seeks\":"<<seeks<<",\"bytes\":"<<bytes<<",\"volume_checks\":"<<volumeChecks<<",\"group_calls\":"<<groupCalls<<",\"initializations\":"<<initializations<<",\"frees\":"<<frees<<",\"sound_files\":"<<names.size()<<",\"manager_states\":"<<managerStates<<",\"manager_lifetimes\":"<<managerLifetimes<<",\"sample_objects\":"<<sampleObjects<<",\"channel_objects\":"<<channelObjects<<",\"api_events\":"<<apiEvents<<"}\n";
        VirtualFree(image,0,MEM_RELEASE);FreeLibrary(reference);
    } catch(const std::exception& error) { std::cerr<<error.what()<<" trial "<<trial<<" step "<<step<<'\n';FreeLibrary(reference);return 1; }
      catch(const char* error) { std::cerr<<error<<'\n';FreeLibrary(reference);return 1; }
    return 0;
}
