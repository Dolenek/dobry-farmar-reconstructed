#include "GameplayTime.h"
#include "StringConversion.h"
#include "OriginalArithmetic.h"
#include <sstream>
#include <stdexcept>
namespace farmar::recovered {
void GameplayTime::loadXML(const WString& text) {
    const auto narrow=toNarrow(text);std::istringstream stream(std::string(narrow.view()));
    stream>>hours;if(stream.get()!=':') throw std::runtime_error("Invalid time format");
    stream>>minutes;if(stream.get()!=':') throw std::runtime_error("Invalid time format");
    stream>>seconds;
}
GameplayTime GameplayTime::fromSeconds(float value) noexcept {
    GameplayTime output;
#if defined(_MSC_VER) && defined(_M_IX86)
    // Preserve the unspilled x87 minute remainder and the two unsigned products.
    // Original 00580f98 truncates to int64 and returns its low DWORD. Changing
    // the control word only for FISTP reproduces that conversion without calling
    // any original program code or depending on a modern CRT helper symbol.
    const float perHour=0.00027777778450399637f,perMinute=0.01666666753590107f,two32=4294967296.f;
    unsigned short savedControl,truncControl;
    std::int64_t converted;
    std::uint32_t component,product;
    __asm {
        fnstcw savedControl
        mov ax,savedControl
        or ax,0c00h
        mov truncControl,ax
        fld value
        fmul perHour
        fldcw truncControl
        fistp converted
        fldcw savedControl
        mov eax,dword ptr converted
        mov component,eax
        imul eax,3600
        mov product,eax
        fild product
        test eax,eax
        jge unsignedHours
        fadd two32
    unsignedHours:
        fsubr value
        fld perMinute
        fmul st(0),st(1)
        fldcw truncControl
        fistp converted
        fldcw savedControl
    }
    output.hours=component;output.minutes=static_cast<std::uint32_t>(converted);
    __asm {
        mov eax,dword ptr converted
        imul eax,60
        mov product,eax
        fild product
        test eax,eax
        jge unsignedMinutes
        fadd two32
    unsignedMinutes:
        fsubr st(0),st(1)
        fldcw truncControl
        fistp converted
        fldcw savedControl
        fstp st(0)
    }
    output.seconds=static_cast<std::uint32_t>(converted);
#else
    output.hours=static_cast<std::uint32_t>(static_cast<std::int64_t>(originalMultiply(value,0.00027777778450399637f)));
    const auto remainder=originalSubtract(value,originalUnsigned(output.hours*3600U));
    output.minutes=static_cast<std::uint32_t>(static_cast<std::int64_t>(originalMultiply(remainder,0.01666666753590107f)));
    output.seconds=static_cast<std::uint32_t>(static_cast<std::int64_t>(originalSubtract(remainder,originalUnsigned(output.minutes*60U))));
#endif
    return output;
}
}
