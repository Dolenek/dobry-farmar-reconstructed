// Actual rejected-input Ogg constructors/destructors on owned initialized storage.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <array>
#include <algorithm>
#include <cstring>
#include <float.h>
#include <stdexcept>
#include "Squall.h"
void need(bool condition,const char* text){if(!condition)throw std::runtime_error(text);}
bool busyAllocation(void* pointer){
    auto heap=GetProcessHeap();need(HeapLock(heap)!=FALSE,"Lock owned process heap");
    PROCESS_HEAP_ENTRY entry{};bool found=false;
    while(HeapWalk(heap,&entry))if(entry.lpData==pointer&&(entry.wFlags&PROCESS_HEAP_ENTRY_BUSY)){found=true;break;}
    need(HeapUnlock(heap)!=FALSE,"Unlock owned process heap");return found;
}
int wmain(int argc,wchar_t**argv){
    if(argc!=8)return 2;
    SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
    std::cout.setf(std::ios::unitbuf);
    try{
        auto source=LoadLibraryW(std::filesystem::absolute(argv[1]).c_str());
        auto native=LoadLibraryW(std::filesystem::absolute(argv[2]).c_str());need(source&&native,"DLL load");
        auto load=reinterpret_cast<decltype(&SQUALL_Sample_LoadFile)>(GetProcAddress(source,"SQUALL_Sample_LoadFile"));
        auto unload=reinterpret_cast<decltype(&SQUALL_Sample_UnloadAll)>(GetProcAddress(source,"SQUALL_Sample_UnloadAll"));need(load&&unload,"Exports");
        auto filename=std::filesystem::path(argv[3]).string();auto id=load(filename.data(),1,nullptr);need(id>0,"Owned PCM WAV load");
        using File=void*(__cdecl*)(int);using Seek=unsigned(__thiscall*)(void*,unsigned);
        auto file=reinterpret_cast<File>(reinterpret_cast<unsigned>(source)+std::stoul(argv[4],nullptr,16));
        auto seek=reinterpret_cast<Seek>(reinterpret_cast<unsigned>(source)+std::stoul(argv[5],nullptr,16));
        auto* owner=file(id);need(owner!=nullptr,"Owned sample");auto* data=*reinterpret_cast<unsigned char**>(static_cast<unsigned char*>(owner)+8);
        auto* reader=*reinterpret_cast<void**>(data+4);unsigned start=*reinterpret_cast<unsigned*>(data+8);
        auto target=std::wstring(argv[6])==L"native"?native:source;
        unsigned ctor=std::stoul(argv[7],nullptr,16);using Ctor=void*(__thiscall*)(void*,WAVEFORMATEX*,bool*,void*);
        unsigned cases=0;std::cout<<"{\"cases\":[";
        for(unsigned precision:{53u,24u})for(bool allocated:{false,true}){
            _control87(precision==24?_PC_24:_PC_53,_MCW_PC);
            need(seek(reader,start)==start,"Reset actual reader");*reinterpret_cast<unsigned*>(data+16)=0;
            alignas(8) std::array<unsigned char,52> object{};std::fill(object.begin()+20,object.end(),0x5a);
            auto seed=allocated?GlobalAlloc(GPTR,4096):nullptr;need(!allocated||seed,"Owned seed allocation");
            if(allocated)need(busyAllocation(seed),"Find actual owned allocation before destructor");
            std::memcpy(object.data()+12,&seed,4);unsigned sizeSeed=0x17295183;std::memcpy(object.data()+16,&sizeSeed,4);
            WAVEFORMATEX format{};bool initialized=true;
            reinterpret_cast<Ctor>(reinterpret_cast<unsigned>(target)+ctor)(object.data(),&format,&initialized,data);
            need(!initialized,"Non-Ogg input accepted");
            unsigned vf,buffer,size;std::memcpy(&vf,object.data()+8,4);std::memcpy(&buffer,object.data()+12,4);std::memcpy(&size,object.data()+16,4);
            need(vf==0&&buffer==reinterpret_cast<unsigned>(seed)&&size==sizeSeed,"Rejected constructor altered originally untouched fields");
            need(std::all_of(object.begin()+20,object.end(),[](unsigned char c){return c==0x5a;}),"Object guard");
            auto** table=*reinterpret_cast<void***>(object.data());using Destroy=void*(__thiscall*)(void*,unsigned);
            reinterpret_cast<Destroy>(table[0])(object.data(),0);
            if(allocated)need(!busyAllocation(seed),"Owned temp allocation remains busy after destructor");
            std::cout<<(cases++?",":"")<<"{\"precision\":"<<precision<<",\"seeded_owned_buffer\":"<<(allocated?"true":"false")<<",\"vf_cleared\":true,\"temp_pointer_untouched_by_constructor\":true,\"temp_size_untouched_by_constructor\":true,\"owned_buffer_freed_by_destructor\":"<<(allocated?"true":"false")<<",\"heap_walk_observed\":true}";
        }
        unload();FreeLibrary(source);FreeLibrary(native);std::cout<<"],\"case_count\":"<<cases<<"}\n";return 0;
    }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}
}
