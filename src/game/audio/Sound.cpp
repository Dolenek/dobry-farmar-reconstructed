#include "Sound.h"
#include "SoundManager.h"
#include <stdexcept>
namespace farmar::recovered {
cSound::cSound(const String& name):cResource(name) {
    auto* manager=SoundManager::instance();if(!manager) throw std::logic_error("SoundManager is not bound");
    sample_=manager->loadResource(name);
}
cSound::~cSound() { if(auto* manager=SoundManager::instance()) manager->unloadResource(sample_); }
}
