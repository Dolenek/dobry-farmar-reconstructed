// Prove the original MPEG2 Layer2 frame-table overread with guarded fixtures.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include <float.h>
#include <filesystem>
#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <cstring>
#include <stdexcept>
using byte=unsigned char;
#define protected public
#include "SoundFile/MP3/MpegDecoder.h"
#undef protected
#include "Squall.h"
using Seek=int(__thiscall*)(void*,unsigned);
struct Fault {unsigned code{},read{},result{};std::uintptr_t address{};};
int filter(EXCEPTION_POINTERS* e,Fault* f){f->code=e->ExceptionRecord->ExceptionCode;if(f->code!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;f->read=unsigned(e->ExceptionRecord->ExceptionInformation[0]);f->address=e->ExceptionRecord->ExceptionInformation[1];return EXCEPTION_EXECUTE_HANDLER;}
void guarded(Seek seek,void* object,unsigned frame,Fault* fault){__try{fault->result=seek(object,frame);}__except(filter(GetExceptionInformation(),fault)) {}}
void need(bool ok,const char* what){if(!ok)throw std::runtime_error(what);}
struct Api {
    HMODULE module{};decltype(&SQUALL_Sample_LoadFile)load{};decltype(&SQUALL_Sample_UnloadAll)unload{};void*(__cdecl*file)(int){};Seek seek{};
    Api(const wchar_t* name,unsigned fileRva,unsigned seekRva){module=LoadLibraryW(std::filesystem::absolute(name).c_str());need(module!=nullptr,"Load DLL");load=reinterpret_cast<decltype(load)>(GetProcAddress(module,"SQUALL_Sample_LoadFile"));unload=reinterpret_cast<decltype(unload)>(GetProcAddress(module,"SQUALL_Sample_UnloadAll"));need(load&&unload,"Exports");file=reinterpret_cast<decltype(file)>(reinterpret_cast<std::uintptr_t>(module)+fileRva);seek=reinterpret_cast<Seek>(reinterpret_cast<std::uintptr_t>(module)+seekRva);}
    ~Api(){if(module){unload();FreeLibrary(module);}}
};
int wmain(int argc,wchar_t**argv){if(argc!=6)return 2;void* allocation{};
    try{
        Api native(argv[1],0x8300,0x126c0),source(argv[2],std::stoul(argv[3],nullptr,16),std::stoul(argv[4],nullptr,16));std::string filename=std::filesystem::path(argv[5]).string();SYSTEM_INFO info{};GetSystemInfo(&info);auto page=info.dwPageSize;
        allocation=VirtualAlloc(nullptr,page*2,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);need(allocation!=nullptr,"Owned guard allocation");DWORD previous{};need(VirtualProtect(static_cast<unsigned char*>(allocation)+page,page,PAGE_NOACCESS,&previous)!=0,"Guard page");
        unsigned cases=0;unsigned overrunPairs=0;std::cout<<"{\"cases\":[";
        for(unsigned precision:{_PC_53,_PC_24})for(int memory:{0,1}){
            _control87(precision,_MCW_PC);auto ns=native.load(filename.data(),memory,nullptr),ss=source.load(filename.data(),memory,nullptr);need(ns>0&&ss==ns,"Load MP2");
            auto* own=*reinterpret_cast<CDecompressMpeg**>(static_cast<unsigned char*>(source.file(ss))+12);auto* original=*reinterpret_cast<CDecompressMpeg**>(static_cast<unsigned char*>(native.file(ns))+12);
            need(own->_frames==10&&original->_frames==10&&own->_samplesInFrame==576&&original->_samplesInFrame==576&&own->_samplesInFile==11520&&original->_samplesInFile==11520,"Actual unmodified metadata");
            need(own->_vbr&&original->_vbr&&!std::memcmp(own->_vbrFrameOffTable,original->_vbrFrameOffTable,40),"Actual ten-entry tables");
            auto* table=reinterpret_cast<DWORD*>(static_cast<unsigned char*>(allocation)+page-40);std::memcpy(table,original->_vbrFrameOffTable,40);
            auto* oldOwn=own->_vbrFrameOffTable;auto* oldNative=original->_vbrFrameOffTable;own->_vbrFrameOffTable=table;original->_vbrFrameOffTable=table;
            Fault a{},b{};guarded(native.seek,original,19,&b);guarded(source.seek,own,19,&a);own->_vbrFrameOffTable=oldOwn;original->_vbrFrameOffTable=oldNative;
            auto expected=reinterpret_cast<std::uintptr_t>(allocation)+page+24;
            need(a.code==EXCEPTION_ACCESS_VIOLATION&&b.code==a.code&&a.read==0&&b.read==0&&a.address==expected&&b.address==expected,"Both actual methods must read table index16 past ten entries");
            using Pcm=unsigned(__thiscall*)(void*,void*,unsigned,unsigned,bool);
            auto ownStereo=reinterpret_cast<Pcm>((*reinterpret_cast<void***>(own))[2]);auto nativeStereo=reinterpret_cast<Pcm>((*reinterpret_cast<void***>(original))[2]);
            std::vector<unsigned char> outputA(16+4608+16,0x5a),outputB=outputA;
            auto returnedA=ownStereo(own,outputA.data()+16,576,577,false),returnedB=nativeStereo(original,outputB.data()+16,576,577,false);
            need(returnedA==2308&&returnedB==2308&&outputA==outputB,"Actual overrun bytes differ");
            unsigned changedTail=0,lastChanged=0;for(unsigned i=0;i<4608;++i)if(outputA[16+i]!=0x5a){lastChanged=i+1;if(i>=2308)++changedTail;}
            need(changedTail>2000&&lastChanged>4590,"Expected original decoded write beyond declared2308 bytes");
            for(unsigned i=0;i<16;++i)need(outputA[i]==0x5a&&outputA[16+4608+i]==0x5a,"Wide backing guard changed");
            ++overrunPairs;
            if(cases++)std::cout<<',';std::cout<<"{\"precision\":"<<precision<<",\"memory\":"<<memory<<",\"entries\":10,\"declared_samples\":11520,\"samples_per_frame\":576,\"requested_frame\":19,\"table_index\":16,\"source_exception\":"<<a.code<<",\"native_exception\":"<<b.code<<",\"guard_byte_offset\":24,\"stereo_requested_frames\":577,\"stereo_returned_bytes\":"<<returnedA<<",\"stereo_changed_bytes_past_declared_end\":"<<changedTail<<",\"stereo_last_changed_byte\":"<<lastChanged<<"}";
            source.unload();native.unload();
        }
        VirtualFree(allocation,0,MEM_RELEASE);allocation=nullptr;std::cout<<"],\"guarded_overreads\":"<<cases*2<<",\"matched_wide_backing_output_overrun_pairs\":"<<overrunPairs<<",\"production_table_mutated\":false,\"diagnostic_table_pointer_temporarily_replaced\":true}\n";return 0;
    }catch(const std::exception&e){if(allocation)VirtualFree(allocation,0,MEM_RELEASE);std::cerr<<e.what()<<'\n';return 1;}
}
