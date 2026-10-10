#include "SoundManager.h"
#include "SoundVolume.h"
#include "Sound.h"
#include "Channel.h"
#include "Settings.h"
#include <stdexcept>
namespace farmar::recovered {
namespace { SoundManager* manager=nullptr;SoundApi* backend=nullptr; }
SoundManager* SoundManager::instance() noexcept { return manager; }
SoundApi& SoundManager::api() { if(!backend) throw std::logic_error("Sound API is not bound");return *backend; }
SoundManager::SoundManager(FileIndex& files,Settings& settings,SoundApi& api,unsigned char initialPause)
    :settings_(settings),api_(api),paused_(initialPause) {
    if(manager) throw "Object already exist.";
    manager=this;backend=&api_;
    try {
        files_=std::make_unique<SoundFiles>(files);
        if(api_.initialize(SoundSetup{})<0) { api_.free();disabled_=true;return; }
        if(api_.setFileCallbacks(&SoundFiles::open,&SoundFiles::seek,&SoundFiles::read,&SoundFiles::close)<0) throw L"Can't setup squall file callbacks.";
        api_.setEaxPreset(3);
        settings_.get(String("Sound"),String("Music"),musicVolume_);
        settings_.get(String("Sound"),String("Sound"),soundVolume_);
        setMusicVolume(musicVolume_);setSoundVolume(soundVolume_);
    } catch(...) { files_.reset();manager=nullptr;throw; }
}
SoundManager::~SoundManager() {
    if(!disabled_) {
        settings_.set(String("Sound"),String("Music"),musicVolume_);
        settings_.set(String("Sound"),String("Sound"),soundVolume_);
        api_.stop();api_.free();
    }
    files_.reset();manager=nullptr;
}
void SoundManager::setSoundVolume(unsigned value) {
    const unsigned transformed=soundGroupVolume(value);soundVolume_=value;transformedSound_=transformed;
    if(!disabled_) api_.setGroupVolume(1,static_cast<int>(transformed));
}
void SoundManager::setMusicVolume(unsigned value) {
    const unsigned transformed=soundGroupVolume(value);musicVolume_=value;transformedMusic_=transformed;
    if(!disabled_) api_.setGroupVolume(2,static_cast<int>(transformed));
}
void SoundManager::setPause(unsigned char value) {
    if(paused_==value) return;paused_=value;if(!disabled_) api_.pause(value);
}
int SoundManager::loadResource(const String& name) {
    if(disabled_) return 0;const int sample=api_.loadSample(name.c_str(),1,0);
    if(sample<0) throw std::runtime_error("Error loading sound file \""+std::string(name.view())+"\"");
    return sample;
}
void SoundManager::unloadResource(int sample) { if(!disabled_) api_.unloadSample(sample); }
cChannel* SoundManager::playSound(cSound* sound,bool loop,bool start) {
    if(disabled_) return nullptr;
    auto* channel=new cChannel(api_.playSample(sound->sampleID(),loop,1,1,0,static_cast<int>(transformedSound_),api_.sampleFrequency(sound->sampleID()),50),sound);
    if(!start) channel->pause();channel->setGroup(1);return channel;
}
void SoundManager::clickSound(cSound* sound) {
    if(!disabled_) api_.playSample(sound->sampleID(),0,1,1,0,static_cast<int>(transformedSound_),api_.sampleFrequency(sound->sampleID()),50);
}
cChannel* SoundManager::playMusic(cSound* sound,bool loop,bool start) {
    if(disabled_) return nullptr;
    auto* channel=new cChannel(api_.playSample(sound->sampleID(),loop,2,start,0,static_cast<int>(transformedMusic_),api_.sampleFrequency(sound->sampleID()),50),sound);
    channel->setGroup(2);return channel;
}
}
