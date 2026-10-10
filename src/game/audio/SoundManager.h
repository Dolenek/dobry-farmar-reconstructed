#pragma once
#include "SoundApi.h"
#include "SoundFiles.h"
namespace farmar::recovered {
class Settings;class cSound;class cChannel;
class SoundManager {
public:
    // Original constructor leaves +4 untouched. A caller supplies that byte;
    // no inferred initial pause value is hidden in this implementation.
    SoundManager(FileIndex& files,Settings& settings,SoundApi& api,unsigned char initialPause); // 00535d50
    virtual ~SoundManager(); // 00535280
    SoundManager(const SoundManager&)=delete;
    SoundManager& operator=(const SoundManager&)=delete;
    virtual cChannel* playSound(cSound* sound,bool loop,bool start); // 00534730
    virtual void clickSound(cSound* sound); // 00534b70
    virtual cChannel* playMusic(cSound* sound,bool loop,bool start); // 00534be0
    virtual bool disabled() const noexcept { return disabled_; } // 00535850
    virtual unsigned soundVolume() const noexcept { return soundVolume_; } // 00532de0
    virtual void setSoundVolume(unsigned volume); // 00532f40
    virtual unsigned musicVolume() const noexcept { return musicVolume_; } // 00532df0
    virtual void setMusicVolume(unsigned volume); // 00532f90
    virtual unsigned char pauseState() const noexcept { return paused_; } // 00535840
    virtual void setPause(unsigned char value); // 00532e00
    int loadResource(const String& name); // 00533620
    void unloadResource(int sample); // 005346e0
    unsigned transformedSoundVolume() const noexcept { return transformedSound_; }
    unsigned transformedMusicVolume() const noexcept { return transformedMusic_; }
    static SoundManager* instance() noexcept;
    static SoundApi& api(); // API outlives SoundManager, as original DLL does.
    SoundFiles& files() noexcept { return *files_; }
private:
    Settings& settings_;SoundApi& api_;std::unique_ptr<SoundFiles> files_;
    unsigned soundVolume_=100,musicVolume_=100,transformedSound_=100,transformedMusic_=100;
    unsigned char paused_;bool disabled_=false;
};
}
