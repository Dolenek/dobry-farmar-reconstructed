#pragma once
#include <array>
namespace farmar::recovered {
bool jpegUsesMMX() noexcept; //005a1b47, CPUID leaf1 EDX bit23
// RGB result. MMX groups use8-bit coefficients/floor; scalar tails use IJG rounding.
std::array<unsigned char,3> jpegYCbCrToRGB(unsigned char y,unsigned char cb,unsigned char cr,bool mmx) noexcept;
}
