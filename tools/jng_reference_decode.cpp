// Read-only Windows oracle. Loads the user's original DLL for pixel comparison.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

int wmain(int argc, wchar_t** argv) {
    if (argc != 4) {
        std::cerr << "Usage: jng-reference-decode original.dll input.jng output.bgra\n";
        return 2;
    }
    static_assert(sizeof(unsigned long) == 4 && sizeof(void*) == 4, "Original x86 ABI");
    const auto dllPath = std::filesystem::absolute(argv[1]);
    HMODULE library = LoadLibraryExW(dllPath.c_str(), nullptr,
        LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    if (!library) {
        std::cerr << "Cannot load original DLL; Windows error " << GetLastError() << "\n";
        return 2;
    }
    using Read = void (__cdecl *)(const void*, unsigned long&, unsigned long&, unsigned long*&);
    using Free = void (__cdecl *)(unsigned long*&);
    const auto read = reinterpret_cast<Read>(GetProcAddress(library, "?readMNG@@YAXPBXAAK1AAPAK@Z"));
    const auto release = reinterpret_cast<Free>(GetProcAddress(library, "?freeData@@YAXAAPAK@Z"));
    if (!read || !release) {
        std::cerr << "Original DLL export ABI differs from reference\n";
        FreeLibrary(library);
        return 2;
    }
    int result = 0;
    try {
        std::ifstream input(std::filesystem::path(argv[2]), std::ios::binary);
        std::vector<unsigned char> bytes{std::istreambuf_iterator<char>(input), {}};
        if (!input || bytes.empty()) throw "Cannot read input";
        bytes.resize(bytes.size() + 64, 0);
        unsigned long width = 0, height = 0;
        unsigned long* pixels = nullptr;
        read(bytes.data(), width, height, pixels);
        std::ofstream output(std::filesystem::path(argv[3]), std::ios::binary);
        output.write(reinterpret_cast<const char*>(pixels), static_cast<std::streamsize>(width) * height * 4);
        release(pixels);
        if (!output) throw "Cannot write pixels";
        std::cout << width << " " << height << "\n";
    } catch (const char* error) {
        std::cerr << error << "\n";
        result = 1;
    } catch (const std::exception& error) {
        std::cerr << error.what() << "\n";
        result = 1;
    }
    FreeLibrary(library);
    return result;
}
