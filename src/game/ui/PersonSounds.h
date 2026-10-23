#pragma once
#include "Resource.h"
#include <array>
#include "Channel.h"
#include <memory>
namespace farmar::recovered {
class SoundManager;
// Original Person sound selection and ownership, using the recovered live
// SoundManager/cChannel backend. The actor controls final teardown ordering.
class PersonSounds final {
public:
    PersonSounds(std::array<cResource*,8>& sounds,SoundManager& manager):sounds_(sounds),manager_(manager) {}
    ~PersonSounds();
    void play(unsigned); // 0044a820
    void stop(unsigned); // 0044ad50, only kind6 has a body
    void stopWatering(); // 00449820
    void stopScythe(); // 00449840
    void stopShovel(); // 00449860
    void stopTractor(); // 00449880
    void clearWorkChannels(); // non-hose channel order in 0044b3a0
    cChannel* channel(unsigned) const noexcept;
private:
    std::array<cResource*,8>& sounds_;SoundManager& manager_;
    std::unique_ptr<cChannel> watering_,hose_,tractor_,shovel_,scythe_;
};
}
