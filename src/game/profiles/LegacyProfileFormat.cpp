#include "LegacyProfileFormat.h"
#include <cstdint>
#include <cstring>
#include <cstdarg>
#include <cstdio>
#include <sstream>
#include <stdexcept>
namespace farmar::recovered {
std::wstring legacyProfileFloatingText(double value) {
    static_assert(sizeof(value)==8,"Original profile Double width");
    std::uint64_t bits{};std::memcpy(&bits,&value,8);
    // Actual 004deaa0/004df4f0 stream output drops the sign of negative zero.
    if((bits&0x7fffffffffffffffULL)==0) return L"0";
    // The original floating stream path quiets signaling NaNs before printf.
    if((bits&0x7ff0000000000000ULL)==0x7ff0000000000000ULL && (bits&0x000fffffffffffffULL)) {
        bits|=0x0008000000000000ULL;std::memcpy(&value,&bits,8);
    }
#if defined(_MSC_VER)
    wchar_t buffer[128]{};
    const auto print=[](wchar_t* output,const wchar_t* spec,...) {
        va_list args;va_start(args,spec);
        const auto count=__stdio_common_vswprintf(_CRT_INTERNAL_PRINTF_LEGACY_MSVCRT_COMPATIBILITY|_CRT_INTERNAL_PRINTF_LEGACY_THREE_DIGIT_EXPONENTS,output,128,spec,nullptr,args);
        va_end(args);return count;
    };
    if(print(buffer,L"%.6g",value)<0) throw std::runtime_error("Profile floating output failed");
    return buffer;
#else
    // Portable rendering of the same spelling; native comparison uses MSVC.
    if((bits&0x7ff0000000000000ULL)==0x7ff0000000000000ULL) {
        const bool negative=(bits>>63)!=0;
        if(!(bits&0x000fffffffffffffULL)) return negative ? L"-1.#INF":L"1.#INF";
        if(bits==0xfff8000000000000ULL) return L"-1.#IND";
        return negative ? L"-1.#QNAN":L"1.#QNAN";
    }
    std::wostringstream out;out.precision(6);out<<value;auto text=out.str();const auto exponent=text.find_first_of(L"eE");
    if(exponent!=text.npos) while(text.size()-exponent-2<3) text.insert(exponent+2,1,L'0');
    return text;
#endif
}
}
