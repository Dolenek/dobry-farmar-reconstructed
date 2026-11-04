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

void need(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
int wmain(int argc, wchar_t** argv) {
    if (argc != 9) return 2;
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
    std::cout.setf(std::ios::unitbuf);
    try {
        _control87(std::stoi(argv[8]) == 24 ? _PC_24 : _PC_53, _MCW_PC);
        auto module = LoadLibraryW(std::filesystem::absolute(argv[1]).c_str());
        need(module != nullptr, "DLL load");
        auto address = [module](const wchar_t* value) {
            return reinterpret_cast<std::uintptr_t>(module) + std::stoul(value, nullptr, 16);
        };
        std::ifstream input(argv[2], std::ios::binary);
        std::vector<unsigned char> stream(std::istreambuf_iterator<char>(input), {});
        need(stream.size() >= 1084, "Fixture length");
        std::vector<unsigned char> object(65536, 0);
        using Create = int(__thiscall*)(void*, const void*, unsigned);
        using Order = void(__thiscall*)(void*);
        using Read = unsigned(__thiscall*)(void*, void*, unsigned, int);
        using Destroy = int(__thiscall*)(void*);
        auto create = reinterpret_cast<Create>(address(argv[3]));
        auto orders = reinterpret_cast<Order>(address(argv[4]));
        auto read = reinterpret_cast<Read>(address(argv[5]));
        auto destroy = reinterpret_cast<Destroy>(address(argv[6]));
        auto save = [&object](const char* name) {
            std::ofstream(name, std::ios::binary).write(reinterpret_cast<char*>(object.data()), object.size());
        };
        auto result = create(object.data(), stream.data(), static_cast<unsigned>(stream.size()));
        save("after-create.bin");
        // The MOD loader uses the old channel field. Preserve actual allocated
        // pattern commands and decoded instrument samples, not pointer values.
        auto word = [&object](unsigned offset) { unsigned value; std::memcpy(&value, object.data()+offset, 4); return value; };
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
        std::cout << "{\"create_result\":" << result;
        need(result != 0, "Actual Create rejected fixture");
        std::string stage = std::filesystem::path(argv[7]).string();
        if (stage == "orders") { orders(object.data()); save("after-orders.bin"); }
        else need(stage == "create", "Stage");
        // No read/predecode is requested here: retain the exact loader and order state.
        (void)read;
        std::cout << ",\"destroy_started\":true";
        auto destroyed = destroy(object.data());
        std::cout << ",\"destroy_result\":" << destroyed << "}\n";
        FreeLibrary(module);
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
