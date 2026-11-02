// Actual sample loaders, DLL decoders and physical buffers for generated WAVs.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include "Squall.h"
#include "SoundEngine/Context.h"
#include <array>
#include <vector>
#include <filesystem>
#include <float.h>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
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
unsigned results{},reads{},loads{},buffers{},currentMemory{},currentChannels{},currentStart{},currentCount{};bool currentLoop{};std::string currentName;
unsigned inputBits{},inputChannels{},inputFrames{};
struct EofObservation {std::string file,kind,source,native;unsigned sourceDecoder,nativeDecoder,start,channels,memory,count;bool equal;};
std::vector<EofObservation> eofObservations;
std::uint64_t pcmBytes{},exactPcmBytes{},physicalBytes{},pcmDigest=14695981039346656037ULL;
void same(int a,int b,const char* what){++results;if(a!=b){std::cerr<<what<<" file="<<currentName<<" memory="<<currentMemory<<" source="<<a<<" native="<<b<<'\n';throw std::runtime_error("WAV API result differs");}}
void save(const char* name,const std::vector<unsigned char>& bytes){std::ofstream out(name,std::ios::binary);out.write(reinterpret_cast<const char*>(bytes.data()),bytes.size());}
std::string hex(const unsigned char* p,unsigned count){const char* digits="0123456789abcdef";std::string out;for(unsigned i=0;i<count;++i){out+=digits[p[i]>>4];out+=digits[p[i]&15];}return out;}
void decoded(Api& s,Api& n,void* own,void* native,unsigned channels,unsigned start,unsigned count,bool loop){
    currentChannels=channels;currentStart=start;currentCount=count;currentLoop=loop;
    std::vector<unsigned char>a(count*channels*2+32,0x5a),b=a;
    const auto left=(channels==1?s.mono:s.stereo)(own,a.data()+16,start,count,loop),right=(channels==1?n.mono:n.stereo)(native,b.data()+16,start,count,loop);
    same(left,right,"Actual decoder return bytes");
    const bool emptyConversion=inputBits&&count&&start>=inputFrames&&inputChannels!=channels;
    if(emptyConversion){
        auto* ownDecoder=*reinterpret_cast<void**>(static_cast<unsigned char*>(own)+12);auto* originalDecoder=*reinterpret_cast<void**>(static_cast<unsigned char*>(native)+12);
        const auto sa=unsigned(reinterpret_cast<std::uintptr_t>(ownDecoder)),na=unsigned(reinterpret_cast<std::uintptr_t>(originalDecoder));
        std::string kind;
        if(inputChannels==1){
            kind=inputBits==8?"saved_this_high_byte":"saved_this_high_word";
            auto expected=[&](unsigned address){std::vector<unsigned char> v(a.size(),0x5a);
                if(inputBits==8)std::memset(v.data()+16,address>>24,count*2);
                else {auto word=static_cast<std::uint16_t>(address>>16);for(unsigned i=0;i<count*4;i+=2)std::memcpy(v.data()+16+i,&word,2);}
                return v;};
            need(a==expected(sa)&&b==expected(na),"Actual-address EOF bytes differ from native saved-this model");
        }else if(inputBits==8){kind="genuinely_uninitialized_stack_bytes_not_claimed_equal";}
        else kind="start_argument_word_reuse";
        eofObservations.push_back({currentName,kind,hex(a.data(),unsigned(a.size())),hex(b.data(),unsigned(b.size())),sa,na,start,channels,currentMemory,count,a==b});
    }
    if(a!=b&&!emptyConversion){save("failed-source-pcm.bin",a);save("failed-native-pcm.bin",b);std::size_t first=0;while(first<a.size()&&a[first]==b[first])++first;
        std::cerr<<"PCM mismatch file="<<currentName<<" memory="<<currentMemory<<" channels="<<channels<<" start="<<start<<" count="<<count<<" loop="<<loop<<" first_byte="<<first<<'\n';throw std::runtime_error("Decoded WAV words/guards differ");}
    for(unsigned i=0;i<16;++i)need(a[i]==0x5a&&a[a.size()-1-i]==0x5a,"PCM allocation guard changed");need(left<=count*channels*2,"Decoder returned beyond requested payload");
    if(emptyConversion&&inputChannels==2&&inputBits==16)need(a==b,"Start-argument EOF bytes differ");
    ++reads;pcmBytes+=left;
    // No address-dependent or uninitialized bytes are normalized into the
    // exact-output digest. Their complete raw pairs are recorded separately.
    if(!emptyConversion||(inputBits==16&&inputChannels==2)){exactPcmBytes+=left;for(unsigned char byte:a){pcmDigest^=byte;pcmDigest*=1099511628211ULL;}}
}
std::vector<unsigned char> buffer(Api& api,int id){
    need(api.context->_channelsArray&&unsigned(id&255)<unsigned(api.context->_channels),"Actual channel object missing");
    auto* b=api.context->_channelsArray[id&255].DS_Buffer;need(b!=nullptr,"Actual DirectSound buffer missing");DSBCAPS caps{};caps.dwSize=sizeof(caps);need(b->GetCaps(&caps)==DS_OK&&caps.dwBufferBytes>0&&caps.dwBufferBytes<1048576,"Buffer caps");
    void *first{},*second{};DWORD firstSize{},secondSize{};need(b->Lock(0,caps.dwBufferBytes,&first,&firstSize,&second,&secondSize,0)==DS_OK,"Lock actual buffer");
    std::vector<unsigned char> bytes(firstSize+secondSize);std::memcpy(bytes.data(),first,firstSize);if(secondSize)std::memcpy(bytes.data()+firstSize,second,secondSize);need(b->Unlock(first,firstSize,second,secondSize)==DS_OK,"Unlock actual buffer");return bytes;
}
void stop(Api& api,int id){need(api.SQUALL_Channel_Stop(id)>=0,"Stop actual WAV channel");const auto end=GetTickCount64()+2000;while(api.SQUALL_Channel_Status(id)!=0){need(GetTickCount64()<end,"WAV worker did not retire");Sleep(1);}}
void physical(Api& s,Api& n,int ss,int ns,bool spatial){
    std::array<float,3> zero{};auto create=[&](Api& api,int sample){return spatial?api.SQUALL_Sample_Play3DEx(sample,1,17,0,zero.data(),zero.data(),7,0,0,1,1000):api.SQUALL_Sample_PlayEx(sample,1,17,0,7,0,0,50);};
    auto nc=create(n,ns),sc=create(s,ss);same(sc,nc,"Real WAV channel");need(nc>=0,"WAV buffer creation failed");auto a=buffer(s,sc),b=buffer(n,nc);
    if(a!=b){save("failed-source-buffer.bin",a);save("failed-native-buffer.bin",b);throw std::runtime_error("Physical WAV PCM differs");}
    ++buffers;physicalBytes+=a.size();stop(s,sc);stop(n,nc);
}
}
int wmain(int argc,wchar_t** argv){
    if(argc<7||argc>8)return 2;HWND window{};
    try{
        std::array<unsigned,4> symbols{};for(unsigned i=0;i<4;++i)symbols[i]=std::stoul(argv[i+3],nullptr,16);
        Api native(argv[1],{0x8300,0x2e020,0x2e060,0x75034}),source(argv[2],symbols);const auto precision=argc==8?_PC_24:_PC_53;_control87(precision,_MCW_PC);
        window=CreateWindowExW(0,L"STATIC",L"Squall WAV formats",0,0,0,64,64,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);need(window!=nullptr,"Create actual window");
        squall_parameters_t p{};p.Window=window;p.SampleRate=44100;p.BitPerSample=16;p.Channels=4;p.UseAlg=1;p.BufferSize=200;p.DistanceFactor=1;p.RolloffFactor=.001f;p.DopplerFactor=1;
        auto result=native.SQUALL_Init(&p);same(source.SQUALL_Init(&p),result,"Actual Init");need(result>=0,"DirectSound Init failed");
        std::ifstream manifest("files.txt");need(bool(manifest),"WAV fixture manifest missing");std::string filename;unsigned frames{},rate{};
        while(manifest>>filename>>frames>>rate){currentName=filename;
            inputBits=filename.rfind("pcm8-",0)==0?8:filename.rfind("pcm16-",0)==0?16:0;inputChannels=inputBits?unsigned(filename[filename.find('-')+1]-'0'):0;inputFrames=frames;
            for(int memory:{0,1}){currentMemory=memory;std::cerr<<"WAV "<<filename<<" memory="<<memory<<'\n';
                int ns=native.SQUALL_Sample_LoadFile(filename.data(),memory,nullptr),ss=source.SQUALL_Sample_LoadFile(filename.data(),memory,nullptr);same(ss,ns,"Load actual WAV");need(ns>0,"Valid WAV fixture rejected");++loads;
                same(source.SQUALL_Sample_GetFileLength(ss),native.SQUALL_Sample_GetFileLength(ns),"WAV frames");need(native.SQUALL_Sample_GetFileLength(ns)==int(frames),"Unexpected native frame count");
                same(source.SQUALL_Sample_GetFileFrequency(ss),native.SQUALL_Sample_GetFileFrequency(ns),"WAV frequency");need(native.SQUALL_Sample_GetFileFrequency(ns)==int(rate),"Unexpected native frequency");
                same(source.SQUALL_Sample_GetFileLengthMs(ss),native.SQUALL_Sample_GetFileLengthMs(ns),"WAV milliseconds");
                auto own=source.file(ss),original=native.file(ns);need(own&&original,"Real decoder objects missing");
                for(unsigned channels:{1U,2U}){
                    decoded(source,native,own,original,channels,0,frames,false);
                    decoded(source,native,own,original,channels,0,1,false);
                    decoded(source,native,own,original,channels,1,7,false);
                    decoded(source,native,own,original,channels,frames/2,17,false);
                    decoded(source,native,own,original,channels,frames-1,9,false);
                    decoded(source,native,own,original,channels,frames,9,false);
                    decoded(source,native,own,original,channels,frames+1,9,false);
                    decoded(source,native,own,original,channels,0,0,false);
                    decoded(source,native,own,original,channels,frames-2,9,true);
                    decoded(source,native,own,original,channels,0,frames+17,true);
                    if(filename.rfind("pcm",0)==0)decoded(source,native,own,original,channels,0xffffffff,7,false);
                }
                physical(source,native,ss,ns,false);physical(source,native,ss,ns,true);
                source.SQUALL_Sample_Unload(ss);native.SQUALL_Sample_Unload(ns);
                auto h=CreateFileA(filename.c_str(),GENERIC_READ,0,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);need(h!=INVALID_HANDLE_VALUE,"Unload retained WAV file handle");CloseHandle(h);
            }
        }
        need(loads>0,"No WAV fixtures tested");need((_control87(0,0)&_MCW_PC)==precision,"Precision changed");source.SQUALL_Free();native.SQUALL_Free();DestroyWindow(window);window=nullptr;
        std::cout<<"{\"result_comparisons\":"<<results<<",\"loaded_samples_per_dll\":"<<loads<<",\"guarded_pcm_reads\":"<<reads<<",\"total_returned_pcm_bytes\":"<<pcmBytes<<",\"exact_equal_pcm_bytes\":"<<exactPcmBytes<<",\"physical_buffers\":"<<buffers<<",\"physical_pcm_bytes\":"<<physicalBytes<<",\"exact_defined_pcm_and_guards_fnv1a64\":\""<<std::hex<<pcmDigest<<std::dec<<"\",\"actual_directsound\":true,\"export_lookups\":194,\"eof_observations\":[";
        for(unsigned i=0;i<eofObservations.size();++i){const auto& r=eofObservations[i];if(i)std::cout<<',';std::cout<<"{\"file\":\""<<r.file<<"\",\"kind\":\""<<r.kind<<"\",\"source_decoder\":"<<r.sourceDecoder<<",\"native_decoder\":"<<r.nativeDecoder<<",\"start\":"<<r.start<<",\"output_channels\":"<<r.channels<<",\"memory\":"<<r.memory<<",\"count\":"<<r.count<<",\"observed_equal\":"<<(r.equal?"true":"false")<<",\"source_raw_hex\":\""<<r.source<<"\",\"native_raw_hex\":\""<<r.native<<"\"}";}std::cout<<"]}\n";return 0;
    }catch(const std::exception& error){if(window)DestroyWindow(window);std::cerr<<error.what()<<" last_file="<<currentName<<" memory="<<currentMemory<<" channels="<<currentChannels<<" start="<<currentStart<<" count="<<currentCount<<" loop="<<currentLoop<<'\n';return 1;}
}
