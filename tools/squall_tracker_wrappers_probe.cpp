// Exercise actual DLL wrapper/Read implementations on explicit empty mixer states.
// Controlled states are not a substitute for decoded-file or playback tests.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <cstdint>
#include <float.h>
#include <stdexcept>
#include <algorithm>
#include <cstring>

void need(bool condition, const char* what) { if (!condition) throw std::runtime_error(what); }
int wmain(int argc, wchar_t** argv) {
    if (argc != 6) return 2;
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
    std::cout.setf(std::ios::unitbuf);
    try {
        auto module = LoadLibraryW(std::filesystem::absolute(argv[1]).c_str());
        need(module != nullptr, "DLL load");
        using Samples = unsigned(__thiscall*)(void*, void*, unsigned, unsigned, bool);
        auto mono = reinterpret_cast<Samples>(reinterpret_cast<std::uintptr_t>(module) + std::stoul(argv[2], nullptr, 16));
        auto stereo = reinterpret_cast<Samples>(reinterpret_cast<std::uintptr_t>(module) + std::stoul(argv[3], nullptr, 16));
        bool legacy = std::wstring(argv[4]) == L"native";
        unsigned shift = 0; // Both reconstructed and native Read use the old view.
        (void)legacy;
        _control87(std::stoi(argv[5]) == 24 ? _PC_24 : _PC_53, _MCW_PC);
        const unsigned precision = _control87(0, 0) & _MCW_PC;
        std::ofstream pcm("destinations.bin", std::ios::binary);
        unsigned cases = 0;
        std::cout << "{\"cases\":[";
        for (unsigned channels : {1u, 2u}) for (unsigned count : {0u, 1u, 2u, 127u, 4096u, 65535u})
        for (unsigned start : {0u, 1u, 0xffffffffu}) for (bool loop : {false, true})
        for (unsigned state : {0u, 1u, 2u}) {
            // state0=no module channels,1=end reached,2=pending empty mix.
            std::vector<unsigned char> object(65536, 0);
            auto set = [&object](unsigned offset, unsigned value) { std::memcpy(object.data()+offset, &value, 4); };
            auto get = [&object](unsigned offset) { unsigned value; std::memcpy(&value,object.data()+offset,4); return value; };
            set(0xd058 + shift, state == 0 ? 0 : 4);
            set(0xd054 + shift, state == 1 ? 0x200 : 0);
            set(0xd064 + shift, count);
            auto before = object;
            std::vector<unsigned char> data(32 + count*channels*2, 0x5a);
            auto result = (channels == 1 ? mono : stereo)(object.data(), data.data()+16, start, count, loop);
            bool guards = true;
            for (unsigned i=0;i<16;++i) guards &= data[i]==0x5a && data[data.size()-1-i]==0x5a;
            bool expected = true;
            unsigned expectedByte = state == 0 ? 0x5a : 0;
            for (unsigned i=16;i<data.size()-16;++i) expected &= data[i] == expectedByte;
            need(guards && expected, "Actual wrapper destination/guard mismatch");
            need(result == (state == 2 ? count : 0), "Actual wrapper frame return mismatch");
            need((_control87(0,0)&_MCW_PC) == precision, "Precision changed");
            for (unsigned i=0;i<object.size();++i)
                need(object[i] == before[i] || (i>=0xd060+shift && i<0xd068+shift), "Unexpected object mutation");
            pcm.write(reinterpret_cast<char*>(data.data()), data.size());
            std::cout << (cases++ ? "," : "") << "{\"channels\":" << channels << ",\"count\":" << count
                      << ",\"start\":" << start << ",\"loop\":" << (loop ? "true" : "false")
                      << ",\"state\":" << state << ",\"returned_frames\":" << result
                      << ",\"remaining_mix_frames\":" << get(0xd064+shift) << "}";
        }
        std::cout << "],\"case_count\":" << cases << ",\"guards_intact\":true,\"precision_unchanged\":true}\n";
        FreeLibrary(module);
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n'; return 1;
    }
}
