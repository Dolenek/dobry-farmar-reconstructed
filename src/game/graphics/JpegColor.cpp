#include "JpegColor.h"
#include <intrin.h>
namespace farmar::recovered {
bool jpegUsesMMX() noexcept {
    static const bool result=[] {int cpu[4]{};__cpuid(cpu,1);return (cpu[3]&0x800000)!=0;}();return result;
}
std::array<unsigned char,3> jpegYCbCrToRGB(unsigned char luminance,unsigned char cbValue,unsigned char crValue,bool mmx) noexcept {
    const int y=luminance,cb=int(cbValue)-128,cr=int(crValue)-128;
    //005a7b11/005a79d4, constants0065f648/50/58. Signed right shift
    // is arithmetic in the required MSVC x86 build, as in PSRAD.
    const int red=y+(mmx ? (359*cr)>>8:(91881*cr+32768)>>16);
    const int green=y+(mmx ? (-88*cb-183*cr)>>8:(-22554*cb-46802*cr+32768)>>16);
    const int blue=y+(mmx ? (454*cb)>>8:(116130*cb+32768)>>16);
    const auto clip=[](int value)->unsigned char {return static_cast<unsigned char>(value<0 ? 0:(value>255 ? 255:value));};
    return {clip(red),clip(green),clip(blue)};
}
}
