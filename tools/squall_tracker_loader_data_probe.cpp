// Actual tracker methods on zeroed, sufficiently large diagnostic storage.
// No patches to loaded DLLs. This does not validate the native factory allocation.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>
#include <string>
#include <float.h>
#include <cstdint>
#include <stdexcept>
#include <cstring>
#include <array>
#include <algorithm>
#include <cstddef>
using byte=unsigned char;
#include "SoundFile/track/stdafx.h"
#include "SoundFile/track/TrackDecoder.h"
static_assert(sizeof(CDecompressTrack)==0xef0c);
static_assert(sizeof(INSTRUMENTHEADER)==0x278);
static_assert(offsetof(CDecompressTrack,legacy_m_lpszSongComments)==0xd0dc);
static_assert(offsetof(CDecompressTrack,legacy_m_MixPlugins)==0xcbc8);
static_assert(offsetof(CDecompressTrack,m_MixPlugins)==0xcbcc);

void need(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
int wmain(int argc, wchar_t** argv) {
    if (argc != 9) return 2;
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
    std::cout.setf(std::ios::unitbuf);
    try {
        const auto requested = std::stoi(argv[8]) == 24 ? _PC_24 : _PC_53;
        auto module = LoadLibraryW(std::filesystem::absolute(argv[1]).c_str());
        need(module != nullptr, "DLL load");
        auto address = [module](const wchar_t* value) {
            return reinterpret_cast<std::uintptr_t>(module) + std::stoul(value, nullptr, 16);
        };
        std::ifstream input(argv[2], std::ios::binary);
        std::vector<unsigned char> stream(std::istreambuf_iterator<char>(input), {});
        need(stream.size() >= 1084, "Fixture length");
        std::vector<unsigned char> backing(65568,0xa7);
        unsigned char* object=backing.data()+16;std::memset(object,0,65536);
        using Create = int(__thiscall*)(void*, const void*, unsigned);
        using Order = void(__thiscall*)(void*);
        using Read = unsigned(__thiscall*)(void*, void*, unsigned, int);
        using Destroy = int(__thiscall*)(void*);
        auto create = reinterpret_cast<Create>(address(argv[3]));
        auto orders = reinterpret_cast<Order>(address(argv[4]));
        auto read = reinterpret_cast<Read>(address(argv[5]));
        auto destroy = reinterpret_cast<Destroy>(address(argv[6]));
        auto save = [&object](const char* name) {
            std::ofstream(name, std::ios::binary).write(reinterpret_cast<char*>(object), 65536);
        };
        _control87(requested,_MCW_PC);const auto createBefore=_control87(0,0);
        auto result = create(object, stream.data(), static_cast<unsigned>(stream.size()));
        const auto createAfter=_control87(0,0);
        save("after-create.bin");
        // The MOD loader uses the old channel field. Preserve actual allocated
        // pattern commands and decoded instrument samples, not pointer values.
        auto word = [&object](unsigned offset) { unsigned value; std::memcpy(&value, object+offset, 4); return value; };
        auto copy = [](const char* name, unsigned pointer, unsigned size) {
            need(size <= 1048576, "Allocated diagnostic extent");
            if (!size) return;
            need(pointer != 0, "Missing allocated tracker data");
            std::vector<unsigned char> bytes(size);
            SIZE_T copied = 0;
            need(ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<const void*>(pointer), bytes.data(), size, &copied) && copied == size, "Read actual tracker allocation");
            std::ofstream(name, std::ios::binary).write(reinterpret_cast<char*>(bytes.data()), size);
        };
        copy("pattern0.bin", word(0x9a08), (word(0x9dc8)&0xffff)*word(0xd058)*6);
        unsigned sample = 0x5148 + 64;
        unsigned flags = word(sample+32) >> 16;
        copy("instrument1-pcm.bin", word(sample+20), word(sample)*((flags&1) ? 2 : 1));
        // Create initializes unused PatternSize entries to64 too. A row
        // count alone is not evidence that the corresponding allocation exists.
        if(word(0x9a0c))copy("pattern1.bin",word(0x9a0c),(word(0x9dca)&0xffff)*word(0xd058)*6);
        if(word(0x8d4c))copy("header1.bin",word(0x8d4c),632);
        // Read669's separately verified allocation is109 bytes. Other loader
        // comment extents require their own bounds before diagnostic copying.
        if(std::filesystem::path(argv[2]).extension()==L".669"&&word(0xd0dc))copy("legacy-comments.bin",word(0xd0dc),109);
        // This fixture has exactly one AMS pattern-name record. Its pointer
        // occupies the old D0E0 slot; compare the allocation, not ASLR addresses.
        if(std::filesystem::path(argv[2]).extension()==L".ams")copy("legacy-pattern-names.bin",word(0xd0e0),32);
        std::cout << "{\"create_result\":" << result << ",\"object_base\":" << reinterpret_cast<std::uintptr_t>(object)
                  << ",\"object_size\":" << sizeof(CDecompressTrack) << ",\"instrument_header_size\":" << sizeof(INSTRUMENTHEADER)
                  << ",\"create_control_before\":" << createBefore << ",\"create_control_after\":" << createAfter;
        need(result != 0, "Actual Create rejected fixture");
        std::string stage = std::filesystem::path(argv[7]).string();
        if (stage == "orders") { _control87(requested,_MCW_PC);const auto before=_control87(0,0);orders(object);const auto after=_control87(0,0);save("after-orders.bin");std::cout<<",\"orders_control_before\":"<<before<<",\"orders_control_after\":"<<after; }
        else need(stage == "create", "Stage");
        // No read/predecode is requested here: retain the exact loader and order state.
        (void)read;
        std::cout << ",\"destroy_started\":true";
        for(unsigned i=0;i<16;++i)need(backing[i]==0xa7&&backing[i+65552]==0xa7,"Owned object outer guard");
        need(std::all_of(object+sizeof(CDecompressTrack),object+65536,[](unsigned char b){return b==0;}),"Loader wrote beyond complete object extent");
        _control87(requested,_MCW_PC);const auto destroyBefore=_control87(0,0);auto destroyed = destroy(object);const auto destroyAfter=_control87(0,0);
        std::cout<<",\"destroy_control_before\":"<<destroyBefore<<",\"destroy_control_after\":"<<destroyAfter<<",\"guards_intact\":true";
        std::cout << ",\"destroy_result\":" << destroyed << "}\n";
        FreeLibrary(module);
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
