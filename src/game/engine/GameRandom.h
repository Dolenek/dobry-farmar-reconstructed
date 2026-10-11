#pragma once
#include <cstdint>
namespace farmar::recovered {
// Shared per-game sequence, recovered from original CRT _rand 00581021.
// Original __getptd starts each thread at 1. Engine startup's seed and every
// consuming world subsystem must use the same sequence when integrated.
class GameRandom final {
public:
    explicit GameRandom(std::uint32_t seed=1) noexcept:state_(seed) {}
    void seed(std::uint32_t value) noexcept {state_=value;}
    std::uint32_t state() const noexcept {return state_;}
    std::int32_t next() noexcept {state_=state_*0x343fdU+0x269ec3U;return (state_>>16)&0x7fff;}
private:
    std::uint32_t state_;
};
}
