#include "PersonSounds.h"
#include "SoundManager.h"
#include "Sound.h"
namespace farmar::recovered {
PersonSounds::~PersonSounds() {stop(6);clearWorkChannels();}
void PersonSounds::play(unsigned kind) {
    const auto sound=[&](unsigned i) {return static_cast<cSound*>(sounds_[i]);};
    // Reset and creation are separate operations: playSound must run after
    // the old wrapper has stopped its channel and released its sound ref.
    switch(kind) {
    case 0:shovel_.reset();shovel_.reset(manager_.playSound(sound(0),true,true));break;
    case 1:tractor_.reset();tractor_.reset(manager_.playSound(sound(1),true,true));break;
    case 2:watering_.reset();watering_.reset(manager_.playSound(sound(2),true,true));break;
    case 3:if(sound(3)) manager_.clickSound(sound(3));break;
    case 4:scythe_.reset();scythe_.reset(manager_.playSound(sound(4),true,true));break;
    case 5:if(sound(5)) manager_.clickSound(sound(5));break;
    case 6:if(!hose_ && sound(6)) hose_.reset(manager_.playSound(sound(6),true,false));if(hose_) hose_->start();break;
    case 7:if(sound(7)) manager_.clickSound(sound(7));break;
    default:break;
    }
}
void PersonSounds::stop(unsigned kind) {if(kind==6) hose_.reset();}
void PersonSounds::stopWatering() {watering_.reset();}
void PersonSounds::stopScythe() {scythe_.reset();}
void PersonSounds::stopShovel() {shovel_.reset();}
void PersonSounds::stopTractor() {tractor_.reset();}
void PersonSounds::clearWorkChannels() {stopWatering();stopScythe();stopTractor();stopShovel();}
cChannel* PersonSounds::channel(unsigned kind) const noexcept {
    switch(kind) {case 0:return shovel_.get();case 1:return tractor_.get();case 2:return watering_.get();case 4:return scythe_.get();case 6:return hose_.get();default:return nullptr;}
}
}
