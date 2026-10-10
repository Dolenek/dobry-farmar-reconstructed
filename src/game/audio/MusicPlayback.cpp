#include "MusicPlayback.h"
#include "Channel.h"
#include "Sound.h"
#include "ResourceManager.h"
#include "SoundManager.h"
namespace farmar::recovered {
MusicPlayback::~MusicPlayback() {clear();}
void MusicPlayback::clear() {
    if(channel_) {delete channel_;channel_=nullptr;}
    if(sound_) {sound_->releaseReference();sound_=nullptr;}
}
void MusicPlayback::replace(ResourceManager& resources,SoundManager& manager,const String& name) {
    clear();sound_=resources.getSound(name);channel_=manager.playMusic(sound_,true,true);
}
}
