#pragma once

// The original decoder computes its tables directly with x87 instructions.
// Keep register values at their actual precision across intermediate steps;
// the current CRT trigonometric functions and double temporaries differ.
namespace native_mpeg {
struct Extended { unsigned char bytes[10]; };

// Original window/window_dual 10010cf0/10010e60 keep every accumulated
// product in an x87 register until _ftol. A float local spills at PC53.
class Accumulator {
    Extended value;
public:
    void reset()
    {
        auto* output = &value;
        __asm {
            fldz
            mov eax, output
            fstp tbyte ptr [eax]
        }
    }
    void addProduct(float coefficient, float sample)
    {
        auto* current = &value;
        __asm {
            mov eax, current
            fld tbyte ptr [eax]
            fld sample
            fmul coefficient
            faddp st(1), st
            fstp tbyte ptr [eax]
        }
    }
    void subtractProduct(float coefficient, float sample)
    {
        auto* current = &value;
        __asm {
            mov eax, current
            fld tbyte ptr [eax]
            fld sample
            fmul coefficient
            fsubp st(1), st
            fstp tbyte ptr [eax]
        }
    }
    long truncate() const
    {
        const auto* input = &value;
        unsigned short original, working;
        __int64 result;
        __asm { fnstcw original }
        working = original | 0x0c00;
        __asm {
            mov eax, input
            fld tbyte ptr [eax]
            fldcw working
            fistp result
            fldcw original
        }
        return static_cast<long>(result);
    }
};

inline void pi(Extended& value)
{
    const double four = 4.0;
    auto* output = &value;
    __asm {
        fld1
        fld1
        fpatan
        fmul four
        mov eax, output
        fstp tbyte ptr [eax]
    }
}

inline void scale(const Extended& input, double factor, Extended& output)
{
    const auto* source = &input;
    auto* destination = &output;
    __asm {
        mov eax, source
        fld tbyte ptr [eax]
        fmul factor
        mov eax, destination
        fstp tbyte ptr [eax]
    }
}

// Original imdct_init 1000e8f0: integer product precedes the angle scale.
inline float cosine(const Extended& angle, int i, int j,
                    bool doubleAngle, bool doubleResult)
{
    float result;
    const auto* source = &angle;
    __asm {
        fild i
        fimul j
        mov eax, source
        fld tbyte ptr [eax]
        fmulp st(1), st
        cmp doubleAngle, 0
        je angle_ready
        fadd st, st
    angle_ready:
        fcos
        cmp doubleResult, 0
        je result_ready
        fadd st, st
    result_ready:
        fstp result
    }
    return result;
}

// Original alias_init 1000abd0 retains the square root between divisions.
inline void alias(float ci, float* output)
{
    __asm {
        fld ci
        fmul st, st
        fld1
        faddp st(1), st
        fsqrt
        fld1
        fdiv st, st(1)
        mov eax, output
        fstp dword ptr [eax]
        fld ci
        fdiv st, st(1)
        fstp dword ptr [eax+4]
        fstp st(0)
    }
}

// Positive finite base and finite exponent are the internal MPEG table domain.
// Original _CIpow 1003cf60 / 1003f3b0 retains an x87 return, selects nearest
// rounding while retaining PC, and restores the caller's control word.
inline void power(double base, double exponent, Extended& result)
{
    auto* output = &result;
    if (base == 0.0) {
        __asm {
            fldz
            mov eax, output
            fstp tbyte ptr [eax]
        }
        return;
    }
    unsigned short original, working;
    __asm { fnstcw original }
    working = (original & 0x0300) | 0x007f;
    __asm {
        fldcw working
        fld exponent
        fld base
        fyl2x
        fld st(0)
        frndint
        fsubr st(1), st
        fxch st(1)
        fchs
        f2xm1
        fld1
        faddp st(1), st
        fscale
        fstp st(1)
        fldcw original
        mov eax, output
        fstp tbyte ptr [eax]
    }
}

inline double powerDouble(double base, double exponent)
{
    Extended value;
    power(base, exponent, value);
    const auto* input = &value;
    double result;
    __asm {
        mov eax, input
        fld tbyte ptr [eax]
        fstp result
    }
    return result;
}

inline float powerFloat(double base, double exponent, double factor = 1.0)
{
    Extended value;
    power(base, exponent, value);
    const auto* input = &value;
    float result;
    __asm {
        mov eax, input
        fld tbyte ptr [eax]
        fmul factor
        fstp result
    }
    return result;
}

// Original long/short dequant branches 1000c5cc/1000c798 multiply the
// retained _CIpow result by xs first, then by the signed integer as double.
inline float dequant(float xs, int sample)
{
    const double signedSample = sample;
    const double magnitude = sample < 0 ? -signedSample : signedSample;
    Extended value;
    power(magnitude, 1.0 / 3.0, value);
    const auto* input = &value;
    float result;
    __asm {
        mov eax, input
        fld tbyte ptr [eax]
        fmul xs
        fmul signedSample
        fstp result
    }
    return result;
}

inline void stereoRatios(const Extended& angle, int index,
                         float* ordinary, float* midside)
{
    const double two = 2.0;
    const auto* input = &angle;
    double sineAndRatio;
    __asm {
        fld two
        fsqrt
        mov eax, input
        fld tbyte ptr [eax]
        fild index
        fmulp st(1), st
        fld st(0)
        fsin
        fstp sineAndRatio
        fcos
        fld st(0)
        fadd sineAndRatio
        fld sineAndRatio
        fdiv st, st(1)
        fst sineAndRatio
        mov eax, ordinary
        fstp dword ptr [eax]
        fxch st(1)
        fdiv st, st(1)
        fxch st(1)
        fstp st(0)
        fst dword ptr [eax+4]
        fld st(1)
        fmul sineAndRatio
        mov eax, midside
        fstp dword ptr [eax]
        fld st(1)
        fmul st, st(1)
        fstp dword ptr [eax+4]
        fstp st(0)
        fstp st(0)
    }
}

inline float squareRootTwo()
{
    const double two = 2.0;
    float result;
    __asm {
        fld two
        fsqrt
        fstp result
    }
    return result;
}

// Original hwin_init 1000ba00 multiplies (i+.5-shift) by pi before
// multiplying by the stored reciprocal. Reassociation changes PC24 bits.
inline float window(const Extended& pi, int index, double shift, double reciprocal)
{
    const auto* input = &pi;
    const double half = 0.5;
    float result;
    __asm {
        fild index
        fadd half
        fsub shift
        mov eax, input
        fld tbyte ptr [eax]
        fmulp st(1), st
        fmul reciprocal
        fsin
        fstp result
    }
    return result;
}
}
