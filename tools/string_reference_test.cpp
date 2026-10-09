// Read-only x86 oracle: map the original EXE without running its entry point.
// Only pure search/compare routines are called, on our own String objects.
// Run via validate_string.py, which verifies the inventoried EXE hash first.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "GameString.h"
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <random>
#include <string>

int wmain(int argc, wchar_t** argv) {
    if (argc != 2) { std::cerr << "Usage: string-reference-test original.exe\n"; return 2; }
    static_assert(sizeof(void*) == 4, "Original x86 ABI");
    using farmar::recovered::String;
    const auto path = std::filesystem::absolute(argv[1]);
    HMODULE module = LoadLibraryExW(path.c_str(), nullptr, DONT_RESOLVE_DLL_REFERENCES);
    if (!module) { std::cerr << "Cannot map original EXE: " << GetLastError() << '\n'; return 2; }
    const auto* image = reinterpret_cast<const unsigned char*>(module);
    using Search = std::uint32_t (__thiscall *)(const String*, const char*, std::uint32_t, std::uint32_t);
    using Compare = int (__thiscall *)(const String*, std::uint32_t, std::uint32_t, const char*, std::uint32_t);
    const auto find = reinterpret_cast<Search>(image + 0x148260);
    const auto rfind = reinterpret_cast<Search>(image + 0x148330);
    const auto compare = reinterpret_cast<Compare>(image + 0x3960);
    std::mt19937 random(0x4641524d);
    unsigned comparisons = 0;
    for (unsigned trial = 0; trial < 20000; ++trial) {
        std::string bytes(random() % 129, '\0');
        for (auto& byte : bytes) byte = static_cast<char>(random() % 256);
        String text{std::string_view(bytes)};
        std::string needle(random() % 12, '\0');
        for (auto& byte : needle) byte = static_cast<char>(random() % 256);
        if (trial % 2 == 0 && !bytes.empty()) {
            const auto offset = random() % bytes.size();
            needle = bytes.substr(offset, std::min<std::size_t>(needle.size(), bytes.size() - offset));
        }
        const auto start = trial % 3 == 0 ? String::npos : random() % 140;
        const auto count = static_cast<std::uint32_t>(needle.size());
        if (text.find(needle.data(), start, count) != find(&text, needle.data(), start, count) ||
            text.rfind(needle.data(), start, count) != rfind(&text, needle.data(), start, count)) {
            std::cerr << "Search mismatch at trial " << trial << '\n'; FreeLibrary(module); return 1;
        }
        comparisons += 2;
        const auto at = random() % (text.size() + 1);
        const auto length = trial % 3 == 0 ? String::npos : random() % 140;
        if (text.compare(at, length, needle.data(), count) != compare(&text, at, length, needle.data(), count)) {
            std::cerr << "Compare mismatch at trial " << trial << '\n'; FreeLibrary(module); return 1;
        }
        ++comparisons;
    }
    FreeLibrary(module);
    std::cout << comparisons << '\n';
    return 0;
}
