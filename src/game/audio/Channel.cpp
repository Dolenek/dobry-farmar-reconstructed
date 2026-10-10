#include "Channel.h"
#include "Sound.h"
#include "SoundManager.h"
#include <stdexcept>
namespace farmar::recovered {
cChannel::cChannel(int channel,cSound* sound):channel_(channel),sound_(static_cast<cSound*>(sound->addReference())) {
    if(channel_<0) throw std::runtime_error("Error sound channel creating. Error code:"+std::to_string(channel_));
}
cChannel::~cChannel() { SoundManager::api().stopChannel(channel_);sound_->releaseReference(); }
void cChannel::pause() {
    auto* manager=SoundManager::instance();if(!manager) throw std::logic_error("SoundManager is not bound");
    if(manager->disabled()) return;
    auto& api=SoundManager::api();
    if(manager->disabled() || api.channelStatus(channel_)!=1) {
        if(manager->disabled() || api.channelStatus(channel_)==2) return;
    }
    api.pauseChannel(channel_,1);
}
void cChannel::start() {
    auto* manager=SoundManager::instance();if(!manager) throw std::logic_error("SoundManager is not bound");
    if(manager->disabled()) return;
    auto& api=SoundManager::api();
    if(manager->disabled() || api.channelStatus(channel_)==2) api.pauseChannel(channel_,0);
    if(!manager->disabled() && api.channelStatus(channel_)==1) return;
    api.startChannel(channel_);
}
}
