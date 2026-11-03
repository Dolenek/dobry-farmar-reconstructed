// Compare actual MPEG decoder methods and real physical buffers in loaded DLLs.
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
unsigned comparisons{},reads{},buffers{},memoryMode{},outputChannels{},readStart{},readCount{};bool readLoop{};std::uint64_t bytes{},bufferBytes{},digest=14695981039346656037ULL;std::string filename;
void same(int a,int b,const char* name){++comparisons;if(a!=b){std::cerr<<name<<" memory="<<memoryMode<<" source="<<a<<" native="<<b<<'\n';throw std::runtime_error("MPEG API differs");}}
void save(const char* name,const std::vector<unsigned char>& data){std::ofstream out(name,std::ios::binary);out.write(reinterpret_cast<const char*>(data.data()),data.size());}
void decoded(Api& s,Api& n,void* own,void* original,unsigned channels,unsigned start,unsigned count,bool loop){
    outputChannels=channels;readStart=start;readCount=count;readLoop=loop;
    std::vector<unsigned char>a(count*channels*2+32,0x5a),b=a;
    auto ar=(channels==1?s.mono:s.stereo)(own,a.data()+16,start,count,loop),br=(channels==1?n.mono:n.stereo)(original,b.data()+16,start,count,loop);same(ar,br,"Decoder byte count");
    if(a!=b){save("failed-source-pcm.bin",a);save("failed-native-pcm.bin",b);unsigned first=0,differences=0;for(unsigned i=0;i<a.size();++i)if(a[i]!=b[i]){if(!differences)first=i;++differences;}
        std::cerr<<"PCM first_byte="<<first<<" different_bytes="<<differences<<'\n';throw std::runtime_error("MPEG PCM/guards differ");}
    for(unsigned i=0;i<16;++i)need(a[i]==0x5a&&a[a.size()-1-i]==0x5a,"MPEG guard changed");need(ar<=count*channels*2,"MPEG exceeded output allocation");++reads;bytes+=ar;
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
    if(argc<8||argc>9)return 2;HWND window{};
    try{
        std::array<unsigned,4> symbols{};for(unsigned i=0;i<4;++i)symbols[i]=std::stoul(argv[i+3],nullptr,16);
        filename=std::filesystem::path(argv[7]).string();Api native(argv[1],{0x8300,0x2e020,0x2e060,0x75034}),source(argv[2],symbols);const auto precision=argc==9?_PC_24:_PC_53;_control87(precision,_MCW_PC);
        window=CreateWindowExW(0,L"STATIC",L"Squall MPEG formats",0,0,0,64,64,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);need(window!=nullptr,"Create window");
        squall_parameters_t p{};p.Window=window;p.SampleRate=44100;p.BitPerSample=16;p.Channels=4;p.UseAlg=1;p.BufferSize=200;p.DistanceFactor=1;p.RolloffFactor=.001f;p.DopplerFactor=1;
        auto init=native.SQUALL_Init(&p);same(source.SQUALL_Init(&p),init,"Init");need(init>=0,"Actual DirectSound Init failed");
        unsigned frames{},frequency{},definedFrames{},tableEntries{},samplesPerFrame{};bool unsafeLayer2Shape=false;
        for(int memory:{0,1}){memoryMode=memory;
            int ns=native.SQUALL_Sample_LoadFile(filename.data(),memory,nullptr),ss=source.SQUALL_Sample_LoadFile(filename.data(),memory,nullptr);same(ss,ns,"Load actual MPEG");need(ns>0,"Original rejected encoded MPEG fixture");
            frames=unsigned(native.SQUALL_Sample_GetFileLength(ns));frequency=unsigned(native.SQUALL_Sample_GetFileFrequency(ns));need(frames>0&&frames<1048576&&frequency>0,"Native metadata invalid");
            same(source.SQUALL_Sample_GetFileLength(ss),frames,"Frame length");same(source.SQUALL_Sample_GetFileFrequency(ss),frequency,"Frequency");same(source.SQUALL_Sample_GetFileLengthMs(ss),native.SQUALL_Sample_GetFileLengthMs(ns),"Milliseconds");
            auto own=source.file(ss),original=native.file(ns);need(own&&original,"Actual decoder object missing");
            const auto* sm=*reinterpret_cast<CDecompressMpeg**>(static_cast<unsigned char*>(own)+12);
            const auto* nm=*reinterpret_cast<CDecompressMpeg**>(static_cast<unsigned char*>(original)+12);
            need(sm->_frames==nm->_frames&&sm->_samplesInFrame==nm->_samplesInFrame&&sm->_samplesInFile==nm->_samplesInFile,"MPEG frame table metadata differs");
            unsafeLayer2Shape=nm->_layer==2&&nm->_version!=1;
            tableEntries=nm->_frames;samplesPerFrame=nm->_samplesInFrame;definedFrames=tableEntries*samplesPerFrame;
            need(definedFrames>0&&definedFrames<=frames,"MPEG defined seek range");
            // Original MPEG2 Layer2 overstates length before correcting its table.
            // Keep and compare that API result and full sequential read; seek only
            // in the actual table domain. Dedicated guard probe proves unsafe tail.
            for(unsigned channels:{1U,2U}){
                decoded(source,native,own,original,channels,0,frames,false);
                decoded(source,native,own,original,channels,0,1,false);
                decoded(source,native,own,original,channels,1,127,false);
                decoded(source,native,own,original,channels,definedFrames/2,unsafeLayer2Shape?2*samplesPerFrame:769,false);
                decoded(source,native,own,original,channels,definedFrames-1,9,false);
                decoded(source,native,own,original,channels,definedFrames,9,false);
                decoded(source,native,own,original,channels,definedFrames+1,9,false);
                decoded(source,native,own,original,channels,0,0,false);
                decoded(source,native,own,original,channels,definedFrames-2,9,true);
                decoded(source,native,own,original,channels,0,frames+17,true);
            }
            if(!unsafeLayer2Shape)for(bool spatial:{false,true}){std::array<float,3> zero{};auto create=[&](Api& api,int sample){return spatial?api.SQUALL_Sample_Play3DEx(sample,1,17,0,zero.data(),zero.data(),7,0,0,1,1000):api.SQUALL_Sample_PlayEx(sample,1,17,0,7,0,0,50);};
                auto nc=create(native,ns),sc=create(source,ss);same(sc,nc,"Actual MPEG channel");need(nc>=0,"MPEG buffer failed");auto a=physical(source,sc),b=physical(native,nc);need(a==b,"Physical MPEG PCM differs");++buffers;bufferBytes+=a.size();stop(source,sc);stop(native,nc);}
            source.SQUALL_Sample_Unload(ss);native.SQUALL_Sample_Unload(ns);
        }
        need((_control87(0,0)&_MCW_PC)==precision,"Precision changed");source.SQUALL_Free();native.SQUALL_Free();DestroyWindow(window);window=nullptr;
        std::cout<<"{\"result_comparisons\":"<<comparisons<<",\"guarded_pcm_reads\":"<<reads<<",\"pcm_bytes\":"<<bytes<<",\"physical_buffers\":"<<buffers<<",\"physical_buffers_skipped_native_layer2_overrun\":"<<(unsafeLayer2Shape?"true":"false")<<",\"physical_pcm_bytes\":"<<bufferBytes<<",\"frames\":"<<frames<<",\"defined_seek_frames\":"<<definedFrames<<",\"frame_table_entries\":"<<tableEntries<<",\"samples_per_frame\":"<<samplesPerFrame<<",\"frequency\":"<<frequency<<",\"pcm_fnv1a64\":\""<<std::hex<<digest<<std::dec<<"\",\"actual_directsound\":true,\"export_lookups\":194}\n";return 0;
    }catch(const std::exception& error){if(window)DestroyWindow(window);std::cerr<<error.what()<<" file="<<filename<<" memory="<<memoryMode<<" channels="<<outputChannels<<" start="<<readStart<<" count="<<readCount<<" loop="<<readLoop<<'\n';return 1;}
}
