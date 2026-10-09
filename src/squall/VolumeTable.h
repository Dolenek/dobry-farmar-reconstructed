#pragma once
#include <array>

namespace farmar::recovered {
// Original SQUALL_Init at 0x10003280 creates these 100 DirectSound values.
std::array<int, 100> makeSquallVolumeTable();
}
