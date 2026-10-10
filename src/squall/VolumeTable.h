#pragma once
#include <array>

namespace farmar::recovered {
// Original SQUALL_Init at 0x10003280 creates these 100 DirectSound values.
std::array<int, 100> makeSquallVolumeTable();
// Inverse arithmetic from Channel_GetVolume 100056b0..100056d7.
// DirectSound returns integer attenuation in [-10000,0]. No added clamp.
int squallVolumeFromAttenuation(int attenuation);
}
