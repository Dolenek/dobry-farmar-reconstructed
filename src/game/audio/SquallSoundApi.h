#pragma once
#include "SoundApi.h"
namespace farmar::recovered {
// The game's fifteen actual imports, linked only to the reconstructed DLL.
// SoundManager owns initialization/Free ordering and the file callback lifetime.
class SquallSoundApi final : public SoundApi {
public:
    int initialize(const SoundSetup&) override;
    void free() override;
    int setFileCallbacks(SoundOpen,SoundSeek,SoundRead,SoundClose) override;
    int setEaxPreset(int) override;
    int pause(int) override;
    int stop() override;
    int setGroupVolume(int,int) override;
    int loadSample(const char*,int,const void*) override;
    void unloadSample(int) override;
    int sampleFrequency(int) override;
    int playSample(int,int,int,int,int,int,int,int) override;
    int stopChannel(int) override;
    int channelStatus(int) override;
    int pauseChannel(int,int) override;
    int startChannel(int) override;
};
}
