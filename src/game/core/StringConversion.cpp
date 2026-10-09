#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "StringConversion.h"
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <stdexcept>

namespace farmar::recovered {
namespace {
char narrowBuffer[256]{}; // 0065fc20, shared across calls
wchar_t wideBuffer[256]{}; // 0065fd20
}
String toNarrow(const WString& text) {
    // Original sprintf is unbounded. Reject overlong input rather than reproducing
    // its buffer overrun; this boundary and non-ASCII CRT behavior need an oracle.
    const auto result = std::snprintf(narrowBuffer, sizeof(narrowBuffer), "%S", text.c_str());
    if (result >= static_cast<int>(sizeof(narrowBuffer))) throw std::length_error("String conversion buffer exceeded");
    if (std::strlen(narrowBuffer) < text.size()) {
        std::memset(narrowBuffer, 0, sizeof(narrowBuffer));
        WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, text.data(), text.size(),
            narrowBuffer, 255, nullptr, nullptr);
    }
    return String(narrowBuffer);
}
WString toWide(const String& text) {
    const auto result = std::swprintf(wideBuffer, 256, L"%S", text.c_str());
    if (result < 0) throw std::runtime_error("Wide String conversion failed");
    return WString(wideBuffer);
}
}
