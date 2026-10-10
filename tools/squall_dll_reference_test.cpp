// Public calls to the real rebuilt DLL and original oracle; no static Squall
// implementation is linked into this executable.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include "Squall.h"
#include "SoundFiles.h"
#include "StringConversion.h"
#include "SoundEngine/SquallApi.h"
#include <filesystem>
#include <float.h>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <array>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstddef>
#include <cstdint>
using namespace farmar::recovered;
namespace {
struct Api {
#define SQUALL_ENTRY(name) decltype(&name) name{};
#include "squall_exports.inc"
#undef SQUALL_ENTRY
    HMODULE module{};
    explicit Api(const wchar_t* path) {
        module=LoadLibraryW(std::filesystem::absolute(path).c_str());
        if(!module) throw std::runtime_error("LoadLibrary failed");
#define SQUALL_ENTRY(name) name=reinterpret_cast<decltype(name)>(GetProcAddress(module,#name));if(!name) throw std::runtime_error(#name);
#include "squall_exports.inc"
#undef SQUALL_ENTRY
    }
    ~Api() {if(module) {SQUALL_Free();FreeLibrary(module);}}
};
void require(bool condition,const char* message) {if(!condition) throw std::runtime_error(message);}
unsigned calls=0;
void same(int actual,int expected,const char* name) {
    ++calls;if(actual!=expected) {std::cerr<<name<<": source "<<actual<<" native "<<expected<<'\n';throw std::runtime_error("Public DLL API result differs");}
}
unsigned sourceSymbol(const Api& source,const std::filesystem::path& dll,const char* name) {
    std::ifstream map(dll.parent_path()/"Squall.map");require(bool(map),"Rebuilt linker map missing");
    const auto* base=reinterpret_cast<unsigned char*>(source.module);
    const auto* dos=reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
    const auto* pe=reinterpret_cast<const IMAGE_NT_HEADERS32*>(base+dos->e_lfanew);
    std::string line;
    unsigned preferred{},timestamp{};
    while(std::getline(map,line)) {
      if(line.find("Preferred load address is ")!=std::string::npos) preferred=std::stoul(line.substr(line.find_last_of(' ')+1),nullptr,16);
      if(line.find("Timestamp is ")!=std::string::npos) {
        const auto start=line.find("Timestamp is ")+13;timestamp=std::stoul(line.substr(start),nullptr,16);
      }
      if(line.find(name)!=std::string::npos) {
        std::istringstream fields(line);std::string segment,symbol,address;
        fields>>segment>>symbol>>address;
        const unsigned flat=std::stoul(address,nullptr,16),rva=flat-preferred;
        require(preferred!=0 && flat>=preferred,"Map preferred base missing");
        require(timestamp==pe->FileHeader.TimeDateStamp,"Linker map and loaded DLL timestamp differ");
        require(rva<pe->OptionalHeader.SizeOfImage,"Map symbol outside rebuilt image");
        return rva;
      }
    }
    throw std::runtime_error(name);
}
std::vector<unsigned char> bufferBytes(context_t* context,int channel) {
    const unsigned index=unsigned(channel)&255U;
    require(context && context->_channelsArray && index<unsigned(context->_channels),"Actual channel allocation missing");
    auto* buffer=context->_channelsArray[index].DS_Buffer;require(buffer!=nullptr,"Actual DirectSound buffer missing");
    DSBCAPS caps{};caps.dwSize=sizeof(caps);require(buffer->GetCaps(&caps)==DS_OK,"DirectSound GetCaps failed");
    require(caps.dwBufferBytes>0 && caps.dwBufferBytes<16*1024*1024,"Invalid physical buffer size");
    void *first{},*second{};DWORD firstSize{},secondSize{};
    require(buffer->Lock(0,caps.dwBufferBytes,&first,&firstSize,&second,&secondSize,0)==DS_OK,"Physical buffer Lock failed");
    std::vector<unsigned char> bytes(firstSize+secondSize);
    std::memcpy(bytes.data(),first,firstSize);
    if(secondSize) std::memcpy(bytes.data()+firstSize,second,secondSize);
    require(buffer->Unlock(first,firstSize,second,secondSize)==DS_OK,"Physical buffer Unlock failed");
    require(bytes.size()==caps.dwBufferBytes,"Incomplete physical buffer snapshot");return bytes;
}
}
int wmain(int argc,wchar_t** argv) {
    if(argc<4 || argc>5) return 2;
    HWND window{};
    try {
        FileIndex files(toNarrow(WString(argv[3])),String("pack"));files.mountCommon(toNarrow(WString(argv[3])));
        SoundFiles callbacks(files);
        Api native(argv[1]),source(argv[2]);
        _control87(argc==5 ? _PC_24:_PC_53,_MCW_PC);
        const auto candidatePath=std::filesystem::absolute(argv[2]);
        auto* ownBase=reinterpret_cast<unsigned char*>(source.module);auto* nativeBase=reinterpret_cast<unsigned char*>(native.module);
        const auto ownFile=reinterpret_cast<void*(__cdecl*)(int)>(ownBase+sourceSymbol(source,candidatePath,"?api_SampleGetFile@@"));
        const auto nativeFile=reinterpret_cast<void*(__cdecl*)(int)>(nativeBase+0x8300);
        using ReadPcm=unsigned(__thiscall*)(void*,void*,unsigned,unsigned,bool);
        const auto ownMono=reinterpret_cast<ReadPcm>(ownBase+sourceSymbol(source,candidatePath,"?GetMonoData@CSoundFile@@"));
        const auto ownStereo=reinterpret_cast<ReadPcm>(ownBase+sourceSymbol(source,candidatePath,"?GetStereoData@CSoundFile@@"));
        const auto nativeMono=reinterpret_cast<ReadPcm>(nativeBase+0x2e020),nativeStereo=reinterpret_cast<ReadPcm>(nativeBase+0x2e060);
        const int devices=native.SQUALL_GetNumDevice();same(source.SQUALL_GetNumDevice(),devices,"GetNumDevice");
        for(int i=0;i<devices;++i) {
            std::array<char,512> a,b;a.fill(0x5a);b=a;
            same(source.SQUALL_GetDeviceName(i,a.data(),int(a.size())),native.SQUALL_GetDeviceName(i,b.data(),int(b.size())),"GetDeviceName");
            require(a==b,"Device name bytes differ");
            squall_device_caps_t c{},d{};
            same(source.SQUALL_GetDeviceCaps(i,&c),native.SQUALL_GetDeviceCaps(i,&d),"GetDeviceCaps");
            require(!std::memcmp(&c,&d,sizeof(c)),"Physical device capability fields differ");
        }
        same(source.SQUALL_SetFileCallbacks(&SoundFiles::open,&SoundFiles::seek,&SoundFiles::read,&SoundFiles::close),
            native.SQUALL_SetFileCallbacks(&SoundFiles::open,&SoundFiles::seek,&SoundFiles::read,&SoundFiles::close),"SetFileCallbacks");
        unsigned samples=0;std::uint64_t dllPcmCalls=0,dllPcmBytes=0,dllPcmFrames=0;
        for(const auto& row:files.files()) {
            const auto& name=row.first;
            if(name.size()<5 || (name.substr(name.size()-4)!=".wav" && name.substr(name.size()-4)!=".ogg")) continue;
            std::string filename=name;
            int expected=native.SQUALL_Sample_LoadFile(filename.data(),1,nullptr);
            int actual=source.SQUALL_Sample_LoadFile(filename.data(),1,nullptr);
            same(actual,expected,"Sample_LoadFile");require(actual>0,"Sample load failed");
            same(source.SQUALL_Sample_GetFileLength(actual),native.SQUALL_Sample_GetFileLength(expected),"Sample_GetFileLength");
            same(source.SQUALL_Sample_GetFileLengthMs(actual),native.SQUALL_Sample_GetFileLengthMs(expected),"Sample_GetFileLengthMs");
            same(source.SQUALL_Sample_GetFileFrequency(actual),native.SQUALL_Sample_GetFileFrequency(expected),"Sample_GetFileFrequency");
            squall_sample_default_t a{},b{};
            same(source.SQUALL_Sample_GetDefault(actual,&a),native.SQUALL_Sample_GetDefault(expected,&b),"Sample_GetDefault");
            require(!std::memcmp(&a,&b,sizeof(a)),"Sample defaults differ");
            const auto actualFile=ownFile(actual),expectedFile=nativeFile(expected);
            require(actualFile && expectedFile,"Actual DLL decoder objects missing");
            const unsigned length=unsigned(native.SQUALL_Sample_GetFileLength(expected));
            std::cerr<<"Actual DLL PCM: "<<filename<<'\n';
            for(unsigned channels=1;channels<=2;++channels) {
                for(unsigned offset=0;offset<length;) {
                    const unsigned count=std::min(4096U,length-offset),start=offset ? 0xffffffffU:0;
                    std::array<unsigned char,16448> decoded{},wanted{};decoded.fill(0x5a);wanted=decoded;
                    const auto readSource=channels==1 ? ownMono:ownStereo,readNative=channels==1 ? nativeMono:nativeStereo;
                    const unsigned returned=readSource(actualFile,decoded.data(),start,count,false),result=readNative(expectedFile,wanted.data(),start,count,false);
                    if(returned!=result || decoded!=wanted || returned!=count*channels*2) {
                        std::cerr<<filename<<" channels="<<channels<<" offset="<<offset<<'\n';
                        throw std::runtime_error("Actual DLL whole PCM differs");
                    }
                    ++dllPcmCalls;dllPcmBytes+=returned;dllPcmFrames+=count;offset+=count;
                }
            }
            source.SQUALL_Sample_Unload(actual);native.SQUALL_Sample_Unload(expected);
            require(callbacks.openCount()==0,"Callback handles leaked");++samples;
        }
        require(samples==45,"Sound dataset changed");
        source.SQUALL_Free();native.SQUALL_Free();
        window=CreateWindowExW(0,L"STATIC",L"Squall source runtime comparison",0,0,0,64,64,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);
        require(window!=nullptr,"Real Win32 window creation failed");
        squall_parameters_t parameters{};
        parameters.Window=window;parameters.SampleRate=44100;parameters.BitPerSample=16;parameters.Channels=16;
        parameters.UseAlg=3;parameters.BufferSize=200;parameters.DistanceFactor=1;parameters.RolloffFactor=.001f;parameters.DopplerFactor=1;
        const int expectedInit=native.SQUALL_Init(&parameters),actualInit=source.SQUALL_Init(&parameters);
        same(actualInit,expectedInit,"Init");require(actualInit>=0,"Both real DirectSound initializations failed");
        same(source.SQUALL_GetDevice(),native.SQUALL_GetDevice(),"GetDevice");
        same(source.SQUALL_Get3DAlgorithm(),native.SQUALL_Get3DAlgorithm(),"Get3DAlgorithm");
        same(source.SQUALL_GetBufferSize(),native.SQUALL_GetBufferSize(),"GetBufferSize");
        same(source.SQUALL_GetSpeakerMode(),native.SQUALL_GetSpeakerMode(),"GetSpeakerMode");
        same(source.SQUALL_GetEAXVersion(),native.SQUALL_GetEAXVersion(),"GetEAXVersion");
        squall_channels_t a{},b{};
        same(source.SQUALL_GetChannelsInfo(&a),native.SQUALL_GetChannelsInfo(&b),"GetChannelsInfo");
        require(!std::memcmp(&a,&b,sizeof(a)),"Initial channel state counts differ");
        same(source.SQUALL_SetFileCallbacks(&SoundFiles::open,&SoundFiles::seek,&SoundFiles::read,&SoundFiles::close),
            native.SQUALL_SetFileCallbacks(&SoundFiles::open,&SoundFiles::seek,&SoundFiles::read,&SoundFiles::close),"SetFileCallbacks after Init");
        std::string music="sounds/menu_music.ogg";
        const int nativeSample=native.SQUALL_Sample_LoadFile(music.data(),1,nullptr),sourceSample=source.SQUALL_Sample_LoadFile(music.data(),1,nullptr);
        same(sourceSample,nativeSample,"Initialized Sample_LoadFile");require(sourceSample>0,"Initialized sample failed");
        const int nativeChannel=native.SQUALL_Sample_PlayEx(nativeSample,1,17,0,7,43,0,50),sourceChannel=source.SQUALL_Sample_PlayEx(sourceSample,1,17,0,7,43,0,50);
        same(sourceChannel,nativeChannel,"Prepared Sample_PlayEx");require(sourceChannel>=0,"Physical buffer preparation failed");
        auto* sourceCtx=reinterpret_cast<context_t*>(ownBase+sourceSymbol(source,candidatePath,"?api@@3Usquall_api_s@@A")+offsetof(squall_api_t,_context));
        auto* nativeCtx=reinterpret_cast<context_t*>(reinterpret_cast<unsigned char*>(native.module)+0x75034);
        unsigned physicalSnapshots=0,physicalBytes=0;
        auto compareBuffer=[&]() {
            const auto decoded=bufferBytes(sourceCtx,sourceChannel),expected=bufferBytes(nativeCtx,nativeChannel);
            require(decoded==expected,"PCM in actual DirectSound buffers differs");
            ++physicalSnapshots;physicalBytes+=unsigned(decoded.size());
        };
        compareBuffer();
        same(source.SQUALL_Channel_Status(sourceChannel),native.SQUALL_Channel_Status(nativeChannel),"Prepared Channel_Status");
        same(source.SQUALL_Channel_GetLength(sourceChannel),native.SQUALL_Channel_GetLength(nativeChannel),"Channel_GetLength");
        same(source.SQUALL_Channel_GetLengthMs(sourceChannel),native.SQUALL_Channel_GetLengthMs(nativeChannel),"Channel_GetLengthMs");
        for(int volume:{0,1,43,99,100,-1,101}) {
            same(source.SQUALL_Channel_SetVolume(sourceChannel,volume),native.SQUALL_Channel_SetVolume(nativeChannel,volume),"Channel_SetVolume");
            same(source.SQUALL_Channel_GetVolume(sourceChannel),native.SQUALL_Channel_GetVolume(nativeChannel),"Channel_GetVolume");
        }
        for(int pan:{0,1,49,50,51,99,100,-1,101}) {
            same(source.SQUALL_Channel_SetPan(sourceChannel,pan),native.SQUALL_Channel_SetPan(nativeChannel,pan),"Channel_SetPan");
            same(source.SQUALL_Channel_GetPan(sourceChannel),native.SQUALL_Channel_GetPan(nativeChannel),"Channel_GetPan");
        }
        for(int frequency:{0,100,22050,44100,88200,-1,100001}) {
            same(source.SQUALL_Channel_SetFrequency(sourceChannel,frequency),native.SQUALL_Channel_SetFrequency(nativeChannel,frequency),"Channel_SetFrequency");
            same(source.SQUALL_Channel_GetFrequency(sourceChannel),native.SQUALL_Channel_GetFrequency(nativeChannel),"Channel_GetFrequency");
        }
        same(source.SQUALL_Channel_GetLoop(sourceChannel),native.SQUALL_Channel_GetLoop(nativeChannel),"Channel_GetLoop");
        same(source.SQUALL_Channel_GetPriority(sourceChannel),native.SQUALL_Channel_GetPriority(nativeChannel),"Channel_GetPriority");
        const int musicLength=native.SQUALL_Channel_GetLength(nativeChannel);
        for(int position:{0,13230,musicLength-100}) {
            same(source.SQUALL_Channel_SetPlayPosition(sourceChannel,position),native.SQUALL_Channel_SetPlayPosition(nativeChannel,position),"Channel_SetPlayPosition");
            compareBuffer();
        }
        same(source.SQUALL_Channel_Stop(sourceChannel),native.SQUALL_Channel_Stop(nativeChannel),"Channel_Stop");
        // Native 100054b0 returns -24 while STAGE=STOP and 0 after the
        // asynchronous worker removes it. Compare terminal state after both
        // workers finish; concurrent DLL calls cannot sample identical epochs.
        const auto deadline=GetTickCount64()+2000;
        unsigned stopPendingSource=0,stopPendingNative=0;
        for(;;) {
            const int actual=source.SQUALL_Channel_Status(sourceChannel),expected=native.SQUALL_Channel_Status(nativeChannel);
            require((actual==0 || actual==-24) && (expected==0 || expected==-24),"Unexpected stop transition state");
            if(actual==0 && expected==0) break;
            if(actual==-24) ++stopPendingSource;if(expected==-24) ++stopPendingNative;
            require(GetTickCount64()<deadline,"Physical stop workers did not retire the channel");Sleep(1);
        }
        same(source.SQUALL_Channel_Status(sourceChannel),native.SQUALL_Channel_Status(nativeChannel),"Stopped Channel_Status");
        source.SQUALL_Sample_Unload(sourceSample);native.SQUALL_Sample_Unload(nativeSample);
        source.SQUALL_Free();native.SQUALL_Free();DestroyWindow(window);window=nullptr;
        require((_control87(0,0)&_MCW_PC)==(argc==5 ? _PC_24:_PC_53),"Precision changed during public DLL tests");
        std::cout<<"{\"export_lookups\":194,\"devices\":"<<devices<<",\"samples\":"<<samples<<",\"api_comparisons\":"<<calls<<",\"init_result\":"<<actualInit<<",\"directsound_initialized\":true,\"prepared_channels\":1,\"physical_snapshots\":"<<physicalSnapshots<<",\"physical_pcm_bytes\":"<<physicalBytes<<",\"dll_pcm_calls\":"<<dllPcmCalls<<",\"dll_pcm_bytes\":"<<dllPcmBytes<<",\"dll_pcm_frames\":"<<dllPcmFrames<<",\"stop_pending_source\":"<<stopPendingSource<<",\"stop_pending_native\":"<<stopPendingNative<<",\"playback_tested\":false}\n";
        return 0;
    } catch(const std::exception& error) {if(window) DestroyWindow(window);std::cerr<<error.what()<<'\n';return 1;}
}
