#pragma once
#include <cstdint>
namespace farmar::recovered {
// cSoundManager 00532f40/00532f90, including approximation 00532e40.
// This precedes the separate volume table inside Squall.dll.
std::uint32_t soundGroupVolume(std::uint32_t userVolume) noexcept;
}
