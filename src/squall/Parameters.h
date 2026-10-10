#pragma once
#include <cstdint>

namespace farmar::recovered {
// Squall.dll 100030a0 copies thirteen words. No game-specific defaults here.
struct SquallParameters {
    std::uint32_t window;
    int device,rate,bits,channels,hardware2D,hardware3D,algorithm;
    int bufferMilliseconds,listenerMode;
    float distanceFactor,rolloffFactor,dopplerFactor;
};
static_assert(sizeof(SquallParameters)==52,"Original Squall parameter ABI");
using SquallDeviceCount=int(__cdecl*)();
// Input may be unreadable (including null); output must hold 52 writable bytes.
// Device enumeration is supplied by the reconstructed engine, never guessed.
void normalizeSquallParameters(const SquallParameters* input,
                              SquallParameters* output,SquallDeviceCount deviceCount);
}
