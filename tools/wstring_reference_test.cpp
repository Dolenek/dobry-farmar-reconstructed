// Test-only pure WString compare routine; no original entry point or initializers.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "GameWString.h"
#include <filesystem>
#include <iostream>
#include <random>
#include <string>

int wmain(int argc, wchar_t** argv) {
    if (argc!=2) return 2;
    const auto module=LoadLibraryExW(std::filesystem::absolute(argv[1]).c_str(), nullptr, DONT_RESOLVE_DLL_REFERENCES);
    if (!module) return 2;
    using farmar::recovered::WString;
    using Compare=int (__thiscall *)(const WString*, unsigned, unsigned, const wchar_t*, unsigned);
    const auto compare=reinterpret_cast<Compare>(reinterpret_cast<unsigned char*>(module)+0x1050);
    std::mt19937 random(0x57464152);
    for (unsigned trial=0; trial<20000; ++trial) {
        std::wstring value(random()%129, 0), needle(random()%30, 0);
        for (auto& unit:value) unit=static_cast<wchar_t>(random()%65536);
        for (auto& unit:needle) unit=static_cast<wchar_t>(random()%65536);
        const auto start=random()%(value.size()+1);
        const auto count=trial%3==0 ? WString::npos : random()%140;
        WString text{std::wstring_view(value)};
        if (text.compare(start, count, needle.data(), needle.size())!=compare(&text, start, count, needle.data(), needle.size())) {
            std::cerr << "UTF-16 comparison mismatch at " << trial << '\n'; return 1;
        }
    }
    FreeLibrary(module);
    std::cout << "20000\n";
    return 0;
}
