// Native routines are test oracles only; no original entry point or Squall
// implementation runs. FileIndex/iFile and Squall API boundaries are explicit.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "SoundFiles.h"
#include "SoundVolume.h"
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
struct FilesAdapter { void** table;FileIndex* files; };
iFile* __fastcall openFile(FilesAdapter* self,void*,const String* name) {
    if(name->view()=="#oracle-null#") return nullptr;
    return self->files->open(*name).release();
}
unsigned initializations=0,frees=0,groupCalls=0;unsigned group=0,volume=0;
int __cdecl initialize(const unsigned* setup) {
    const unsigned expected[]={0,0,44100,16,16,0,0,3,200,0,0x3f800000,0x3a83126f,0x3f800000};
    require(!std::memcmp(setup,expected,sizeof(expected)),"Sound initialization parameters differ");++initializations;return -1;
}
void __cdecl freeSound() { ++frees; }
int __cdecl groupVolume(unsigned nextGroup,unsigned nextVolume) { group=nextGroup;volume=nextVolume;++groupCalls;return -73; }
}
int wmain(int argc,wchar_t** argv) {
    if(argc!=3 && argc!=4) return 2;
    auto reference=LoadLibraryExW(argv[1],nullptr,DONT_RESOLVE_DLL_REFERENCES);if(!reference) return 2;
    try {
        if(argc==4) _control87(_PC_24,_MCW_PC);AddVectoredExceptionHandler(1,&diagnose);
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
        redirect(0x15739a,reinterpret_cast<void*>(&freeSound));redirect(0x15737c,reinterpret_cast<void*>(&groupVolume));
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
        std::cout<<"{\"lifetimes\":32,\"opens\":"<<opens<<",\"missing\":"<<missing<<",\"reads\":"<<reads<<",\"seeks\":"<<seeks<<",\"bytes\":"<<bytes<<",\"volume_checks\":"<<volumeChecks<<",\"group_calls\":"<<groupCalls<<",\"initializations\":"<<initializations<<",\"frees\":"<<frees<<",\"sound_files\":"<<names.size()<<"}\n";
        VirtualFree(image,0,MEM_RELEASE);FreeLibrary(reference);
    } catch(const std::exception& error) { std::cerr<<error.what()<<" trial "<<trial<<" step "<<step<<'\n';FreeLibrary(reference);return 1; }
      catch(const char* error) { std::cerr<<error<<'\n';FreeLibrary(reference);return 1; }
    return 0;
}
