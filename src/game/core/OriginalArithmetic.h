#pragma once
// The original Win32 engine performs these operations with x87. Direct3D8's
// CreateDevice flags 0x20 change its precision control to 24 bits; MSVC's modern
// SSE double expressions otherwise retain 53 bits between original spill points.
// Execute elementary arithmetic under the current x87 control word, so both
// default diagnostic precision and the real device context are represented.
namespace farmar::recovered {
inline double originalAdd(double lhs,double rhs) noexcept {
#if defined(_MSC_VER) && defined(_M_IX86)
    double result;
    __asm {
        fld lhs
        fadd rhs
        fstp result
    }
    return result;
#else
    return lhs+rhs;
#endif
}
inline double originalSubtract(double lhs,double rhs) noexcept {
#if defined(_MSC_VER) && defined(_M_IX86)
    double result;
    __asm {
        fld lhs
        fsub rhs
        fstp result
    }
    return result;
#else
    return lhs-rhs;
#endif
}
inline double originalMultiply(double lhs,double rhs) noexcept {
#if defined(_MSC_VER) && defined(_M_IX86)
    double result;
    __asm {
        fld lhs
        fmul rhs
        fstp result
    }
    return result;
#else
    return lhs*rhs;
#endif
}
inline double originalDivide(double lhs,double rhs) noexcept {
#if defined(_MSC_VER) && defined(_M_IX86)
    double result;
    __asm {
        fld lhs
        fdiv rhs
        fstp result
    }
    return result;
#else
    return lhs/rhs;
#endif
}
inline double originalUnsigned(unsigned value) noexcept {
    const auto signedValue=static_cast<int>(value);
    return signedValue<0 ? originalAdd(signedValue,4294967296.0):double(signedValue);
}
}
