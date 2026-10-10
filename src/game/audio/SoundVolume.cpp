#include "SoundVolume.h"
#include <cstring>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error SoundManager arithmetic requires the original Win32 x87 environment
#endif
namespace farmar::recovered {
std::uint32_t soundGroupVolume(std::uint32_t userVolume) noexcept {
    const int signedVolume=static_cast<int>(userVolume);
    const float unsignedCorrection=4294967296.0f, scale=0.01f;
    const float correction=0.47f, coefficient=1.47f, three=3.0f, half=0.5f, percent=100.0f;
    float input,seed,first;
    unsigned bits;
    unsigned short originalControl,truncationControl;
    __int64 integer;
    // Retain x87 precision until the original float argument spill.
    __asm {
        fild signedVolume
    }
    if(signedVolume<0) { __asm { fadd unsignedCorrection } }
    __asm {
        fmul scale
        fstp input
    }
    std::memcpy(&bits,&input,4);
    const int seedBits=static_cast<int>(0xbe800000U-bits)>>1;
    std::memcpy(&seed,&seedBits,4);
    // Squaring seed before multiplying .47*x matters in pc24 precision.
    __asm {
        fld correction
        fmul input
        fld seed
        fld st(0)
        fmul st,st(1)
        fld coefficient
        fxch st(3)
        fmulp st(1),st
        fsubp st(2),st
        fmulp st(1),st
        fstp first
    }
    std::memcpy(&bits,&first,4);bits&=0x7fffffffU;std::memcpy(&first,&bits,4);
    // The second correction and multiplication by 100 have no float spill.
    __asm {
        fld first
        fmul first
        fmul input
        fsubr three
        fmul first
        fmul half
        fmul input
        fmul percent
        fnstcw originalControl
    }
    truncationControl=originalControl|0x0c00;
    __asm {
        fldcw truncationControl
        fistp integer
        fldcw originalControl
    }
    return static_cast<std::uint32_t>(integer);
}
}
