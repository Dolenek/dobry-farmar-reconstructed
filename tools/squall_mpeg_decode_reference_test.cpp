// Compare actual MPEG decoder methods without opening an audio device or creating a window.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include "Squall.h"
using byte=unsigned char;
#define protected public
#include "SoundFile/MP3/MpegDecoder.h"
#undef protected
#include "SoundEngine/Context.h"
#include <array>
#include <vector>
#include <filesystem>
#include <float.h>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
namespace {
void need(bool ok,const char* text){if(!ok)throw std::runtime_error(text);}
using Pcm=unsigned(__thiscall*)(void*,void*,unsigned,unsigned,bool);
struct Api {
#define SQUALL_ENTRY(name) decltype(&name) name{};
#include "squall_exports.inc"
#undef SQUALL_ENTRY
    HMODULE module{};context_t* context{};void*(__cdecl* file)(int){};Pcm mono{},stereo{};
    Api(const wchar_t* path,const std::array<unsigned,4>& symbols){module=LoadLibraryW(std::filesystem::absolute(path).c_str());need(module!=nullptr,"Load DLL");
#define SQUALL_ENTRY(name) name=reinterpret_cast<decltype(name)>(GetProcAddress(module,#name));need(name!=nullptr,#name);
#include "squall_exports.inc"
#undef SQUALL_ENTRY
        auto base=reinterpret_cast<std::uintptr_t>(module);file=reinterpret_cast<decltype(file)>(base+symbols[0]);mono=reinterpret_cast<Pcm>(base+symbols[1]);stereo=reinterpret_cast<Pcm>(base+symbols[2]);context=reinterpret_cast<context_t*>(base+symbols[3]);
    }
    ~Api(){if(module){SQUALL_Free();SQUALL_Sample_UnloadAll();FreeLibrary(module);}}
};
unsigned comparisons{},reads{},buffers{},memoryMode{},outputChannels{},readStart{},readCount{};bool readLoop{};std::uint64_t nonzeroPcmBytes{},bytes{},bufferBytes{},digest=14695981039346656037ULL;std::string filename;
void same(int a,int b,const char* name){++comparisons;if(a!=b){std::cerr<<name<<" memory="<<memoryMode<<" source="<<a<<" native="<<b<<'\n';throw std::runtime_error("MPEG API differs");}}
void save(const char* name,const std::vector<unsigned char>& data){std::ofstream out(name,std::ios::binary);out.write(reinterpret_cast<const char*>(data.data()),data.size());}
void decoded(Api& s,Api& n,void* own,void* original,unsigned channels,unsigned start,unsigned count,bool loop){
    outputChannels=channels;readStart=start;readCount=count;readLoop=loop;
    std::vector<unsigned char>a(count*channels*2+32,0x5a),b=a;
    auto ar=(channels==1?s.mono:s.stereo)(own,a.data()+16,start,count,loop),br=(channels==1?n.mono:n.stereo)(original,b.data()+16,start,count,loop);same(ar,br,"Decoder byte count");
    if(a!=b){save("failed-source-pcm.bin",a);save("failed-native-pcm.bin",b);unsigned first=0,differences=0;for(unsigned i=0;i<a.size();++i)if(a[i]!=b[i]){if(!differences)first=i;++differences;}
        std::cerr<<"PCM first_byte="<<first<<" different_bytes="<<differences<<'\n';throw std::runtime_error("MPEG PCM/guards differ");}
    for(unsigned i=0;i<16;++i)need(a[i]==0x5a&&a[a.size()-1-i]==0x5a,"MPEG guard changed");need(ar<=count*channels*2,"MPEG exceeded output allocation");++reads;bytes+=ar;for(unsigned i=0;i<ar;++i)nonzeroPcmBytes+=a[i+16]!=0;
    for(unsigned char value:a){digest^=value;digest*=1099511628211ULL;}
}
std::vector<unsigned char> physical(Api& api,int id){
    need(api.context->_channelsArray&&unsigned(id&255)<unsigned(api.context->_channels),"Actual channel missing");auto* buffer=api.context->_channelsArray[id&255].DS_Buffer;need(buffer!=nullptr,"Real buffer missing");
    DSBCAPS caps{};caps.dwSize=sizeof(caps);need(buffer->GetCaps(&caps)==DS_OK&&caps.dwBufferBytes>0&&caps.dwBufferBytes<1048576,"GetCaps");
    void *first{},*second{};DWORD firstSize{},secondSize{};need(buffer->Lock(0,caps.dwBufferBytes,&first,&firstSize,&second,&secondSize,0)==DS_OK,"Lock");
    std::vector<unsigned char> data(firstSize+secondSize);std::memcpy(data.data(),first,firstSize);if(secondSize)std::memcpy(data.data()+firstSize,second,secondSize);need(buffer->Unlock(first,firstSize,second,secondSize)==DS_OK,"Unlock");return data;
}
void stop(Api& api,int id){need(api.SQUALL_Channel_Stop(id)>=0,"Stop");auto end=GetTickCount64()+2000;while(api.SQUALL_Channel_Status(id)!=0){need(GetTickCount64()<end,"MPEG worker did not retire");Sleep(1);}}
}
int wmain(int argc,wchar_t** argv){
    if(argc<8||argc>9)return 2;SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);HWND window{};
    try{
        std::array<unsigned,4> symbols{};for(unsigned i=0;i<4;++i)symbols[i]=std::stoul(argv[i+3],nullptr,16);
        filename=std::filesystem::path(argv[7]).string();Api native(argv[1],{0x8300,0x2e020,0x2e060,0x75034}),source(argv[2],symbols);const auto precision=argc==9?_PC_24:_PC_53;_control87(precision,_MCW_PC);
        unsigned layer{},version{},frames{},frequency{},definedFrames{},tableEntries{},samplesPerFrame{};bool unsafeLayer2Shape=false;
        for(int memory:{0,1}){memoryMode=memory;
            int ns=native.SQUALL_Sample_LoadFile(filename.data(),memory,nullptr),ss=source.SQUALL_Sample_LoadFile(filename.data(),memory,nullptr);same(ss,ns,"Load actual MPEG");need(ns>0,"Original rejected encoded MPEG fixture");
            frames=unsigned(native.SQUALL_Sample_GetFileLength(ns));frequency=unsigned(native.SQUALL_Sample_GetFileFrequency(ns));need(frames>0&&frames<1048576&&frequency>0,"Native metadata invalid");
            same(source.SQUALL_Sample_GetFileLength(ss),frames,"Frame length");same(source.SQUALL_Sample_GetFileFrequency(ss),frequency,"Frequency");same(source.SQUALL_Sample_GetFileLengthMs(ss),native.SQUALL_Sample_GetFileLengthMs(ns),"Milliseconds");
            auto own=source.file(ss),original=native.file(ns);need(own&&original,"Actual decoder object missing");
            const auto* sm=*reinterpret_cast<CDecompressMpeg**>(static_cast<unsigned char*>(own)+12);
            const auto* nm=*reinterpret_cast<CDecompressMpeg**>(static_cast<unsigned char*>(original)+12);
            need(sm->_frames==nm->_frames&&sm->_samplesInFrame==nm->_samplesInFrame&&sm->_samplesInFile==nm->_samplesInFile,"MPEG frame table metadata differs");
            layer=nm->_layer;version=nm->_version;need(sm->_layer==layer&&sm->_version==version,"MPEG header layer/version differs");unsafeLayer2Shape=layer==2&&version!=1;
            tableEntries=nm->_frames;samplesPerFrame=nm->_samplesInFrame;definedFrames=tableEntries*samplesPerFrame;
            need(layer==1&&samplesPerFrame>=384&&frames/samplesPerFrame>=5,"LayerI defined frame-prefix range");definedFrames=frames<definedFrames?frames:definedFrames;
            // Native HeaderInfo reports1152/576 samples for LayerI too, while
            // L1decode_frame writes384. Its public size also predates the
            // corrected frame table. Compare only actually written prefixes;
            // do not read or normalize the original uninitialized gap.
            const unsigned safe=definedFrames/samplesPerFrame;
            for(unsigned channels:{1U,2U})for(unsigned frame:{0U,safe/4,safe/2,safe-2,safe-1}) {
                decoded(source,native,own,original,channels,frame*samplesPerFrame,384,false);
                decoded(source,native,own,original,channels,frame*samplesPerFrame+1,127,true);
            }
            source.SQUALL_Sample_Unload(ss);native.SQUALL_Sample_Unload(ns);
        }
        need((_control87(0,0)&_MCW_PC)==precision,"Precision changed");source.SQUALL_Free();native.SQUALL_Free();window=nullptr;
        std::cout<<"{\"result_comparisons\":"<<comparisons<<",\"layer\":"<<layer<<",\"version\":"<<version<<",\"nonzero_pcm_bytes\":"<<nonzeroPcmBytes<<",\"guarded_pcm_reads\":"<<reads<<",\"pcm_bytes\":"<<bytes<<",\"physical_buffers\":"<<buffers<<",\"physical_buffers_skipped_native_layer2_overrun\":"<<(unsafeLayer2Shape?"true":"false")<<",\"physical_pcm_bytes\":"<<bufferBytes<<",\"frames\":"<<frames<<",\"defined_seek_frames\":"<<definedFrames<<",\"frame_table_entries\":"<<tableEntries<<",\"samples_per_frame\":"<<samplesPerFrame<<",\"frequency\":"<<frequency<<",\"pcm_fnv1a64\":\""<<std::hex<<digest<<std::dec<<"\",\"actual_directsound\":false,\"export_lookups\":194}\n";return 0;
    }catch(const std::exception& error){if(window)DestroyWindow(window);std::cerr<<error.what()<<" file="<<filename<<" memory="<<memoryMode<<" channels="<<outputChannels<<" start="<<readStart<<" count="<<readCount<<" loop="<<readLoop<<'\n';return 1;}
}
