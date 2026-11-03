// Read actual constructed decoder state; diagnostic only, no state writes.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include <array>
#include <vector>
#include <filesystem>
#include <float.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
using byte=unsigned char; // Legacy header normally obtains this SDK typedef via non-lean windows.h.
#define protected public
#include "SoundFile/MP3/MpegDecoder.h"
#undef protected
#undef min
#undef max
#include "Squall.h"
namespace {
void need(bool ok,const char* text){if(!ok)throw std::runtime_error(text);}
struct Api {HMODULE module{};decltype(&SQUALL_Sample_LoadFile) load{};decltype(&SQUALL_Sample_UnloadAll) unload{};void*(__cdecl* file)(int){};
    Api(const wchar_t* path,unsigned rva){module=LoadLibraryW(std::filesystem::absolute(path).c_str());need(module!=nullptr,"Load DLL");load=reinterpret_cast<decltype(load)>(GetProcAddress(module,"SQUALL_Sample_LoadFile"));unload=reinterpret_cast<decltype(unload)>(GetProcAddress(module,"SQUALL_Sample_UnloadAll"));need(load&&unload,"API");file=reinterpret_cast<decltype(file)>(reinterpret_cast<std::uintptr_t>(module)+rva);}
    ~Api(){if(module){unload();FreeLibrary(module);}}
};
unsigned emitted{};
void array(const char* name,const unsigned char* source,const unsigned char* native,unsigned offset,unsigned count){
    if(emitted++)std::cout<<',';unsigned different=0;
    for(unsigned i=0;i<count;++i){std::uint32_t a,b;std::memcpy(&a,source+offset+i*4,4);std::memcpy(&b,native+offset+i*4,4);different+=a!=b;}
    std::cout<<"{\"field\":\""<<name<<"\",\"offset\":"<<offset<<",\"words\":"<<count<<",\"different\":"<<different<<",\"first_differences\":[";
    unsigned shown=0;for(unsigned i=0;i<count&&shown<8;++i){std::uint32_t a,b;std::memcpy(&a,source+offset+i*4,4);std::memcpy(&b,native+offset+i*4,4);if(a!=b){if(shown++)std::cout<<',';std::cout<<'['<<i<<','<<a<<','<<b<<']';}}
    std::cout<<"]}";
}
}
int wmain(int argc,wchar_t** argv){if(argc<5||argc>6)return 2;
    try{
        Api native(argv[1],0x8300),source(argv[2],std::stoul(argv[3],nullptr,16));_control87(argc==6?_PC_24:_PC_53,_MCW_PC);std::string filename=std::filesystem::path(argv[4]).string();
        auto ns=native.load(filename.data(),0,nullptr),ss=source.load(filename.data(),0,nullptr);need(ns>0&&ss==ns,"Load actual MPEG");
        const auto* own=*reinterpret_cast<unsigned char**>(static_cast<unsigned char*>(source.file(ss))+12);const auto* original=*reinterpret_cast<unsigned char**>(static_cast<unsigned char*>(native.file(ns))+12);
        auto dump=[](const char* path,const unsigned char* ptr){std::ofstream out(path,std::ios::binary);out.write(reinterpret_cast<const char*>(ptr),sizeof(CDecompressMpeg));};dump("source-decoder.bin",own);dump("native-decoder.bin",original);
        std::cout<<"{\"decoder_bytes\":"<<sizeof(CDecompressMpeg)<<",\"source_decoder\":"<<reinterpret_cast<std::uintptr_t>(own)<<",\"native_decoder\":"<<reinterpret_cast<std::uintptr_t>(original)<<",\"fields\":[";
#define FIELD(name) array(#name,own,original,offsetof(CDecompressMpeg,name),sizeof(((CDecompressMpeg*)nullptr)->name)/4)
        FIELD(m_sf_table);FIELD(m_look_c_valueL2);FIELD(m_c_value);FIELD(m_cs_factor);FIELD(csa);FIELD(lr);FIELD(lr2);FIELD(win);FIELD(look_global);FIELD(look_scale);FIELD(look_pow);FIELD(look_subblock);FIELD(w);FIELD(w2);FIELD(coef);FIELD(v);FIELD(v2);FIELD(coef87);FIELD(coef32);FIELD(vbuf);FIELD(vbuf2);FIELD(m_sample);FIELD(m_yout);FIELD(m_sbt_wrk);
#undef FIELD
        const auto* sm=reinterpret_cast<const CDecompressMpeg*>(own);const auto* nm=reinterpret_cast<const CDecompressMpeg*>(original);
        std::cout<<"],\"mpeg_metadata\":{";
#define META(name) std::cout<<"\"" #name "\":{\"offset\":"<<offsetof(CDecompressMpeg,name)<<",\"source\":"<<+sm->name<<",\"native\":"<<+nm->name<<"},"
        META(_frames);META(_samplesInFrame);META(_samplesInFile);META(_curFrame);META(_vbr);META(_bitPerFrame);
#undef META
        std::cout<<"\"frame_offsets\":[";need(sm->_frames==nm->_frames&&sm->_frames<10000,"Frame metadata");for(unsigned i=0;i<sm->_frames;++i){if(i)std::cout<<',';std::cout<<'['<<sm->_vbrFrameOffTable[i]<<','<<nm->_vbrFrameOffTable[i]<<']';}
        std::cout<<"]}}\n";return 0;
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
