// Whole editable runtime versus the original DLL, as a test oracle only.
// Both decode through the same real recovered read-only FileIndex callbacks.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include "Squall.h"
#include "SoundEngine/SquallApi.h"
#include "SoundFiles.h"
#include "StringConversion.h"
#include <array>
#include <cstring>
#include <filesystem>
#include <float.h>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstdint>
#include <map>
using namespace farmar::recovered;
namespace {
template<class T> T exported(HMODULE module,const char* name) {
    const auto function=GetProcAddress(module,name);if(!function) throw std::runtime_error(name);
    return reinterpret_cast<T>(function);
}
void require(bool condition,const char* error) { if(!condition) throw std::runtime_error(error); }
}
int wmain(int argc,wchar_t** argv) {
    if(argc<3 || argc>4) return 2;
    HMODULE original=LoadLibraryW(std::filesystem::absolute(argv[1]).c_str());if(!original) return 2;
    _control87(argc==4 ? _PC_24 : _PC_53,_MCW_PC);
    try {
        const auto nativeDevices=exported<int(__cdecl*)()>(original,"SQUALL_GetNumDevice");
        const auto nativeSetFiles=exported<decltype(&SQUALL_SetFileCallbacks)>(original,"SQUALL_SetFileCallbacks");
        const auto nativeLoad=exported<int(__cdecl*)(const char*,int,squall_sample_default_t*)>(original,"SQUALL_Sample_LoadFile");
        const auto nativeUnload=exported<void(__cdecl*)(int)>(original,"SQUALL_Sample_Unload");
        const auto nativeFree=exported<void(__cdecl*)()>(original,"SQUALL_Free");
        const auto nativeLength=exported<int(__cdecl*)(int)>(original,"SQUALL_Sample_GetFileLength");
        const auto nativeMilliseconds=exported<int(__cdecl*)(int)>(original,"SQUALL_Sample_GetFileLengthMs");
        const auto nativeFrequency=exported<int(__cdecl*)(int)>(original,"SQUALL_Sample_GetFileFrequency");
        const auto nativeDefaults=exported<int(__cdecl*)(int,squall_sample_default_t*)>(original,"SQUALL_Sample_GetDefault");
        const auto nativeFile=reinterpret_cast<void*(__cdecl*)(int)>(reinterpret_cast<unsigned char*>(original)+0x8300);
        const auto nativeMono=reinterpret_cast<unsigned(__thiscall*)(void*,void*,unsigned,unsigned,bool)>(reinterpret_cast<unsigned char*>(original)+0x2e020);
        const auto nativeStereo=reinterpret_cast<unsigned(__thiscall*)(void*,void*,unsigned,unsigned,bool)>(reinterpret_cast<unsigned char*>(original)+0x2e060);
        const int devices=nativeDevices(),sourceDevices=SQUALL_GetNumDevice();
        require(sourceDevices==devices,"Actual DirectSound device count differs");
        FileIndex files(toNarrow(WString(argv[2])),String("pack"));files.mountCommon(toNarrow(WString(argv[2])));
        SoundFiles callbacks(files);
        require(nativeSetFiles(&SoundFiles::open,&SoundFiles::seek,&SoundFiles::read,&SoundFiles::close)==
                SQUALL_SetFileCallbacks(&SoundFiles::open,&SoundFiles::seek,&SoundFiles::read,&SoundFiles::close),"File callback setup result differs");
        std::vector<String> names;
        for(const auto& file:files.files()) if(file.first.size()>4 && (file.first.substr(file.first.size()-4)==".wav" || file.first.substr(file.first.size()-4)==".ogg")) names.emplace_back(file.first.c_str());
        require(names.size()==45,"Original sound dataset differs");
        unsigned metadata=0,pcmCalls=0,pcmBytes=0;
        std::uint64_t sequentialCalls=0,sequentialBytes=0,sequentialFrames=0;
        std::map<unsigned,unsigned> decoders;
        for(const auto& name:names) {
            std::cerr<<"Comparing "<<name.c_str()<<'\n';
            std::string sourceName(name.c_str());
            const int expected=nativeLoad(name.c_str(),1,nullptr),actual=SQUALL_Sample_LoadFile(sourceName.data(),1,nullptr);
            require(expected>0 && actual>0,"Original sound file load failed");
            require(actual==expected,"Sample allocation ID differs");
            const int length=nativeLength(expected);
            require(length==SQUALL_Sample_GetFileLength(actual),"Sample frame length differs");++metadata;
            require(nativeMilliseconds(expected)==SQUALL_Sample_GetFileLengthMs(actual),"Sample millisecond length differs");++metadata;
            require(nativeFrequency(expected)==SQUALL_Sample_GetFileFrequency(actual),"Sample frequency differs");++metadata;
            squall_sample_default_t actualDefaults{},expectedDefaults{};
            require(nativeDefaults(expected,&expectedDefaults)==SQUALL_Sample_GetDefault(actual,&actualDefaults) &&
                    !std::memcmp(&actualDefaults,&expectedDefaults,sizeof(actualDefaults)),"Sample default fields differ");++metadata;
            auto* source=api_SampleGetFile(actual);auto* reference=nativeFile(expected);
            require(source && reference,"Sample decode objects missing");
            const auto decoder=*reinterpret_cast<void**>(static_cast<unsigned char*>(reference)+12);
            ++decoders[reinterpret_cast<unsigned>(*reinterpret_cast<void**>(decoder))-reinterpret_cast<unsigned>(original)];
            // Initial, middle and final windows in both channel conversions.
            // Full sequential PCM and streaming/loop/error cases follow this audit.
            for(unsigned offset:{0U,unsigned(length/2),unsigned(length>2048 ? length-2048:0)}) {
                const unsigned count=std::min(1024U,unsigned(length)-offset);
                for(unsigned channels=1;channels<=2;++channels) {
                    std::array<unsigned char,8192> decoded{},wanted{};decoded.fill(0x5a);wanted=decoded;
                    const unsigned returned=channels==1 ? source->GetMonoData(decoded.data(),offset,count,false):source->GetStereoData(decoded.data(),offset,count,false);
                    const unsigned result=channels==1 ? nativeMono(reference,wanted.data(),offset,count,false):nativeStereo(reference,wanted.data(),offset,count,false);
                    if(returned!=result || decoded!=wanted) {
                        unsigned difference=0;while(difference<decoded.size() && decoded[difference]==wanted[difference]) ++difference;
                        std::cerr<<"PCM differs: "<<name.c_str()<<" channels "<<channels<<" offset "<<offset<<" byte "<<difference<<" source return "<<returned<<" native return "<<result<<'\n';
                        if(difference+2<decoded.size()) {
                            short a,b;std::memcpy(&a,decoded.data()+(difference&~1U),2);std::memcpy(&b,wanted.data()+(difference&~1U),2);
                            std::cerr<<"Signed output: "<<a<<" native "<<b<<" format channels "<<source->GetFormat()->nChannels<<" bits "<<source->GetFormat()->wBitsPerSample<<'\n';
                            const auto decoder=*reinterpret_cast<void**>(static_cast<unsigned char*>(reference)+12);
                            std::cerr<<"Native decoder vtable RVA: "<<std::hex<<reinterpret_cast<unsigned>(*reinterpret_cast<void**>(decoder))-reinterpret_cast<unsigned>(original)<<std::dec<<'\n';
                            std::array<short,4096> stereo{},nativeStereoBytes{};
                            source->GetStereoData(stereo.data(),offset,count,false);nativeStereo(reference,nativeStereoBytes.data(),offset,count,false);
                            const unsigned sample=difference/2;
                            std::cerr<<"Stereo input: "<<stereo[sample*2]<<','<<stereo[sample*2+1]<<" native "<<nativeStereoBytes[sample*2]<<','<<nativeStereoBytes[sample*2+1]<<'\n';
                        }
                        throw std::runtime_error("Decoded PCM differs");
                    }
                    ++pcmCalls;pcmBytes+=returned;
                }
            }
            for(unsigned channels=1;channels<=2;++channels) {
                for(unsigned offset=0;offset<unsigned(length);) {
                    const unsigned count=std::min(4096U,unsigned(length)-offset);
                    const unsigned start=offset ? 0xffffffffU:0;
                    std::array<unsigned char,16448> decoded{},wanted{};decoded.fill(0x5a);wanted=decoded;
                    const unsigned returned=channels==1 ? source->GetMonoData(decoded.data(),start,count,false):source->GetStereoData(decoded.data(),start,count,false);
                    const unsigned result=channels==1 ? nativeMono(reference,wanted.data(),start,count,false):nativeStereo(reference,wanted.data(),start,count,false);
                    if(returned!=result || decoded!=wanted || result!=count*channels*2) {
                        unsigned difference=0;while(difference<decoded.size() && decoded[difference]==wanted[difference]) ++difference;
                        std::cerr<<"Sequential PCM differs: "<<name.c_str()<<" channels "<<channels<<" offset "<<offset<<" byte "<<difference<<" source return "<<returned<<" native return "<<result<<'\n';
                        throw std::runtime_error("Whole decoded PCM differs");
                    }
                    ++sequentialCalls;sequentialBytes+=returned;sequentialFrames+=count;offset+=count;
                }
            }
            nativeUnload(expected);SQUALL_Sample_Unload(actual);
            require(callbacks.openCount()==0,"Sound file callback lifetime leaked");
        }
        nativeFree();SQUALL_Free();FreeLibrary(original);
        require((_control87(0,0)&_MCW_PC)==(argc==4 ? _PC_24:_PC_53),"x87 precision changed during comparison");
        std::cout<<"{\"devices\":"<<devices<<",\"files\":"<<names.size()<<",\"metadata\":"<<metadata<<",\"pcm_calls\":"<<pcmCalls<<",\"pcm_bytes\":"<<pcmBytes<<",\"sequential_calls\":"<<sequentialCalls<<",\"sequential_bytes\":"<<sequentialBytes<<",\"sequential_frames\":"<<sequentialFrames<<",\"decoders\":{";
        bool first=true;for(const auto& row:decoders) {if(!first) std::cout<<',';first=false;std::cout<<'\"'<<std::hex<<row.first<<std::dec<<"\":"<<row.second;}
        std::cout<<"}}\n";return 0;
    } catch(const std::exception& error) {
        std::cerr<<error.what()<<'\n';SQUALL_Free();exported<void(__cdecl*)()>(original,"SQUALL_Free")();FreeLibrary(original);return 1;
    }
}
