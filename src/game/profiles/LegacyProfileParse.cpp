#include "LegacyProfileParse.h"
#include "GameWString.h"
#include <sstream>
#include <cstring>
#include <stdexcept>
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
namespace farmar::recovered {
namespace {
#if defined(_WIN32)
bool convertDecimal(const std::string& token,double& value) {
    //0057fdc3 calls the old CRT strtod, not the modern UCRT conversion.
    // Use the Windows runtime from System32; no original game module is loaded.
    // In particular its rounding at DBL_MIN differs from the modern converter.
    struct Runtime {
        HMODULE module=LoadLibraryExW(L"msvcrt.dll",nullptr,LOAD_LIBRARY_SEARCH_SYSTEM32);
        using Convert=double(__cdecl*)(const char*,char**);
        using Error=int*(__cdecl*)();
        Convert convert=module ? reinterpret_cast<Convert>(GetProcAddress(module,"strtod")):nullptr;
        Error error=module ? reinterpret_cast<Error>(GetProcAddress(module,"_errno")):nullptr;
        Runtime() { if(!convert||!error) throw std::runtime_error("Windows legacy numeric runtime unavailable"); }
    };
    static const Runtime runtime;
    int* status=runtime.error();const int previous=*status;*status=0;
    char* tail=nullptr;value=runtime.convert(token.c_str(),&tail);
    const int code=*status;*status=previous;
    return !code && tail!=token.c_str();
}
#endif
struct DecimalField {std::string token;int scale=0;};
DecimalField gather(const WString& text) {
    //004d8510: C-locale _Getffld keeps36 significant digits, drops leading
    // fractional zeros into a separate decimal scale, and gathers8 exponent
    // digits. In particular .00123 becomes .0123 with scale=-1.
    DecimalField result;const auto field=text.view();std::size_t at=0;
    const auto digit=[&]{return at<field.size()&&field[at]>=L'0'&&field[at]<=L'9';};
    while(at<field.size()&&(field[at]==L' '||field[at]==L'\t'||field[at]==L'\r'||field[at]==L'\n'||field[at]==L'\v'||field[at]==L'\f')) ++at;
    if(at<field.size()&&(field[at]==L'+'||field[at]==L'-')) result.token.push_back(static_cast<char>(field[at++]));
    unsigned significant=0;bool any=false;
    while(digit()) {
        const char c=static_cast<char>(field[at++]);any=true;
        if(significant<36) {if(c!='0'||significant) {result.token.push_back(c);++significant;}}
        else ++result.scale;
    }
    if(any&&!significant) result.token.push_back('0');
    if(at<field.size()&&field[at]==L'.') {result.token.push_back('.');++at;}
    if(!significant) {
        while(at<field.size()&&field[at]==L'0') {--result.scale;++at;any=true;}
        if(result.scale<0) {result.token.push_back('0');++result.scale;}
    }
    while(digit()) {if(significant<36) {result.token.push_back(static_cast<char>(field[at]));++significant;}++at;any=true;}
    if(any&&at<field.size()&&(field[at]==L'e'||field[at]==L'E')) {
        result.token.push_back('e');++at;
        if(at<field.size()&&(field[at]==L'+'||field[at]==L'-')) result.token.push_back(static_cast<char>(field[at++]));
        bool zeros=false;while(at<field.size()&&field[at]==L'0') {zeros=true;++at;}
        if(zeros) result.token.push_back('0');
        unsigned count=0;bool exponent=zeros;
        while(digit()) {if(count++<8) result.token.push_back(static_cast<char>(field[at]));++at;exponent=true;}
        any=exponent;
    }
    if(!any) result.token.clear();return result;
}
bool extract(const WString& text,double& value) {
    if(text.empty()) return false; //004d95c0/004d9a90 skip extraction entirely
    const auto field=gather(text);if(field.token.empty()) return false;
#if defined(_WIN32)
    if(!convertDecimal(field.token,value)) return false;
#else
    std::istringstream stream{field.token};stream>>value;if(stream.fail()) return false;
#endif
    std::uint64_t bits{};std::memcpy(&bits,&value,8);
    // Actual old num_get reports ERANGE for Double subnormals and leaves the
    // destination intact. Float narrowing occurs only after this check.
    if((bits&0x7ff0000000000000ULL)==0 && (bits&0x000fffffffffffffULL)!=0) return false;
    //0057fdc3 scales after strtod at the caller's x87 precision. These
    // multiplications do not call strtod again or set its range error.
#if defined(_MSC_VER) && defined(_M_IX86)
    const double factor=field.scale<0 ? 0.1:10.0;int count=field.scale<0 ? -field.scale:field.scale;
    double scaledValue=value;
    __asm {
        fld scaledValue
        mov ecx,count
        test ecx,ecx
        jz scaled
    scaling:
        fmul factor
        dec ecx
        jnz scaling
    scaled:
        fstp scaledValue
    }
    value=scaledValue;
#else
    for(int n=field.scale;n>0;--n) value*=10.0;
    for(int n=field.scale;n<0;++n) value*=0.1;
#endif
    return true;
}
}
std::uint32_t parseLegacyProfileFloat(const WString& text,std::uint32_t destination) {
    double value{};
    if(extract(text,value)) {
        // Native extraction first converts Double, then narrows. Finite
        // DBL_MAX becomes Float infinity; Float subnormal narrowing is allowed.
        float narrowed;
#if defined(_MSC_VER) && defined(_M_IX86)
        __asm {
            fld value
            fstp narrowed
        }
#else
        narrowed=static_cast<float>(value);
#endif
        std::memcpy(&destination,&narrowed,4);
    }
    return destination;
}
std::uint64_t parseLegacyProfileDouble(const WString& text,std::uint64_t destination) {
    double value{};if(extract(text,value)) std::memcpy(&destination,&value,8);
    //004eb403/004eb406 copy the retained Double via FLD/FSTP, quieting sNaN.
#if defined(_MSC_VER) && defined(_M_IX86)
    __asm {
        fld qword ptr destination
        fstp qword ptr destination
    }
#else
    if((destination&0x7ff0000000000000ULL)==0x7ff0000000000000ULL && (destination&0x000fffffffffffffULL)) destination|=0x0008000000000000ULL;
#endif
    return destination;
}
#if defined(_MSC_VER) && defined(_M_IX86)
__declspec(naked) std::uint64_t captureProfileDoubleScratch() {
    //004eb390 reserves an uninitialized8-byte local. Snapshot real own stack
    // words through assembly, avoiding a C++ read of an indeterminate Double.
    __asm {
        sub esp,8
        mov eax,[esp]
        mov edx,[esp+4]
        add esp,8
        ret
    }
}
#else
std::uint64_t captureProfileDoubleScratch() {
    throw std::runtime_error("Original Double scratch ABI requires MSVC Win32");
}
#endif
}
