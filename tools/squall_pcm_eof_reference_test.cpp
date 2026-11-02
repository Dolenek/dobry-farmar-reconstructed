// Same-address decoder fixtures distinguish native saved-register/argument
// reuse from genuinely uninitialized input. No production DLL state is patched.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include "Squall.h"
#include "SoundFile/WAV/PcmDecoder.h"
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
static_assert(sizeof(CDecompressPcm8Unsigned)==8 && sizeof(CDecompressPcm16Signed)==16,"Native decoder copy lengths");
void need(bool ok,const char* text){if(!ok)throw std::runtime_error(text);}
using Pcm=unsigned(__thiscall*)(void*,void*,unsigned,unsigned,bool);
struct Api {
#define SQUALL_ENTRY(name) decltype(&name) name{};
#include "squall_exports.inc"
#undef SQUALL_ENTRY
    HMODULE module{};void*(__cdecl* file)(int){};
    Api(const wchar_t* path,unsigned rva){module=LoadLibraryW(std::filesystem::absolute(path).c_str());need(module!=nullptr,"Load DLL");
#define SQUALL_ENTRY(name) name=reinterpret_cast<decltype(name)>(GetProcAddress(module,#name));need(name!=nullptr,#name);
#include "squall_exports.inc"
#undef SQUALL_ENTRY
        file=reinterpret_cast<decltype(file)>(reinterpret_cast<std::uintptr_t>(module)+rva);
    }
    ~Api(){if(module){SQUALL_Sample_UnloadAll();SQUALL_Free();FreeLibrary(module);}}
};
std::string hex(const unsigned char* p,unsigned count){const char* digits="0123456789abcdef";std::string out;for(unsigned i=0;i<count;++i){out+=digits[p[i]>>4];out+=digits[p[i]&15];}return out;}
struct Row {unsigned kind,address,memory,start,loop,sourceResult,nativeResult;std::string source,native,expected;};
std::vector<Row> rows;unsigned mismatches{};
}
int wmain(int argc,wchar_t** argv){
    if(argc<4||argc>5)return 2;
    try{
        Api native(argv[1],0x8300),source(argv[2],std::stoul(argv[3],nullptr,16));const auto precision=argc==5?_PC_24:_PC_53;_control87(precision,_MCW_PC);
        const bool diagnose=GetEnvironmentVariableW(L"FARMAR_PCM_EOF_DIAGNOSE",nullptr,0)!=0;std::vector<unsigned> addresses;
        for(auto base:{0x21000000U,0x4b000000U,0x73000000U}){
            auto* region=static_cast<unsigned char*>(VirtualAlloc(reinterpret_cast<void*>(base),65536,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));if(!region)continue;
            auto* fixture=region+0x1234;addresses.push_back(unsigned(reinterpret_cast<std::uintptr_t>(fixture)));
            for(unsigned kind=0;kind<3;++kind)for(unsigned memory:{0U,1U}){
                char eight[]="pcm8-1.wav",sixteenMono[]="pcm16-1.wav",sixteenStereo[]="pcm16-2.wav";auto* filename=kind==0?eight:kind==1?sixteenMono:sixteenStereo;
                const auto ns=native.SQUALL_Sample_LoadFile(filename,memory,nullptr),ss=source.SQUALL_Sample_LoadFile(filename,memory,nullptr);need(ns>0&&ss==ns,"Load EOF source samples");
                auto* nativeDecoder=*reinterpret_cast<unsigned char**>(static_cast<unsigned char*>(native.file(ns))+12);auto* ownDecoder=*reinterpret_cast<unsigned char**>(static_cast<unsigned char*>(source.file(ss))+12);
                const unsigned objectBytes=kind==0?8:16,slot=kind==2?1:2;
                const auto original=reinterpret_cast<Pcm>((*reinterpret_cast<void***>(nativeDecoder))[slot]),own=reinterpret_cast<Pcm>((*reinterpret_cast<void***>(ownDecoder))[slot]);
                const unsigned frames=native.SQUALL_Sample_GetFileLength(ns);need(frames==256,"Unexpected EOF fixture frames");
                for(unsigned start:{frames,frames+1,0xffffffffU})for(bool loop:{false,true}){
                    const unsigned payload=kind==0?18:kind==1?36:18;std::array<unsigned char,80>a,b,expected;a.fill(0x5a);b=a;expected=a;
                    if(kind==0)std::memset(expected.data()+16,unsigned(reinterpret_cast<std::uintptr_t>(fixture))>>24,payload);
                    else {
                        short value{};
                        if(kind==1){auto bits=static_cast<std::uint16_t>(reinterpret_cast<std::uintptr_t>(fixture)>>16);std::memcpy(&value,&bits,2);}
                        else {std::array<short,2> words{};std::memcpy(words.data(),&start,4);value=short((int(words[0])+int(words[1]))>>1);}
                        for(unsigned i=0;i<payload;i+=2)std::memcpy(expected.data()+16+i,&value,2);
                    }
                    std::memcpy(fixture,nativeDecoder,objectBytes);const auto nr=original(fixture,b.data()+16,start,9,loop);
                    need(nr==payload&&b==expected,"Native EOF register/argument model differs");
                    std::memcpy(fixture,ownDecoder,objectBytes);const auto sr=own(fixture,a.data()+16,start,9,loop);
                    const bool same=sr==nr&&a==b;if(!same)++mismatches;
                    rows.push_back({kind,unsigned(reinterpret_cast<std::uintptr_t>(fixture)),memory,start,unsigned(loop),sr,nr,hex(a.data()+16,payload),hex(b.data()+16,payload),hex(expected.data()+16,payload)});
                    if(!same&&!diagnose)throw std::runtime_error("Source EOF bytes differ from same-address native fixture");
                }
                source.SQUALL_Sample_Unload(ss);native.SQUALL_Sample_Unload(ns);
            }
            VirtualFree(region,0,MEM_RELEASE);
        }
        need(addresses.size()>=2,"Fewer than two controlled addresses available");need((_control87(0,0)&_MCW_PC)==precision,"Precision changed");
        std::cout<<"{\"cases\":"<<rows.size()<<",\"source_mismatches\":"<<mismatches<<",\"native_expected_model_verified\":true,\"original_entry_point_executed\":true,\"export_lookups\":194,\"controlled_addresses\":[";
        for(unsigned i=0;i<addresses.size();++i){if(i)std::cout<<',';std::cout<<addresses[i];}std::cout<<"],\"observations\":[";
        for(unsigned i=0;i<rows.size();++i){const auto& r=rows[i];if(i)std::cout<<',';std::cout<<"{\"kind\":"<<r.kind<<",\"address\":"<<r.address<<",\"memory\":"<<r.memory<<",\"start\":"<<r.start<<",\"loop\":"<<r.loop<<",\"source_return\":"<<r.sourceResult<<",\"native_return\":"<<r.nativeResult<<",\"source_hex\":\""<<r.source<<"\",\"native_hex\":\""<<r.native<<"\",\"expected_hex\":\""<<r.expected<<"\"}";}std::cout<<"]}\n";return 0;
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
