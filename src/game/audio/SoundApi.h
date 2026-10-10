#pragma once
#include <cstdint>
namespace farmar::recovered {
struct SoundSetup {
    unsigned window=0;int device=0,rate=44100,bits=16,channels=16,hardware2D=0,hardware3D=0,algorithm=3,bufferMilliseconds=200,listenerMode=0;
    float distanceFactor=1.0f,rolloffFactor=0.001f,dopplerFactor=1.0f;
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
