#pragma once
#include <cstdint>
#include "../../squall/Parameters.h"
namespace farmar::recovered {
struct SoundSetup : SquallParameters {
    SoundSetup():SquallParameters{0,0,44100,16,16,0,0,3,200,0,1.0f,0.001f,1.0f} {}
};
static_assert(sizeof(SoundSetup)==52,"Original Squall Init parameter ABI");
using SoundOpen=unsigned(__cdecl*)(const char*);
using SoundSeek=int(__cdecl*)(unsigned,int,int);
using SoundRead=int(__cdecl*)(unsigned,void*,int);
using SoundClose=void(__cdecl*)(unsigned);
// Every operation is required. Diagnostic implementations belong only in tools;
// production needs the reconstructed Squall implementation, not original DLLs.
class SoundApi {
public:
    virtual ~SoundApi()=default;
    virtual int initialize(const SoundSetup& setup)=0;
    virtual void free()=0;
    virtual int setFileCallbacks(SoundOpen,SoundSeek,SoundRead,SoundClose)=0;
    virtual int setEaxPreset(int)=0;
    virtual int pause(int)=0;
    virtual int stop()=0;
    virtual int setGroupVolume(int,int)=0;
    virtual int loadSample(const char*,int,const void* defaults)=0;
    virtual int unloadSample(int)=0;
    virtual int sampleFrequency(int)=0;
    virtual int playSample(int,int,int,int,int,int,int,int)=0;
    virtual int stopChannel(int)=0;
    virtual int channelStatus(int)=0;
    virtual int pauseChannel(int,int)=0;
    virtual int startChannel(int)=0;
};
}
