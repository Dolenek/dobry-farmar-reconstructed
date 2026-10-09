// Native diagnostic exercises the reconstructed iFile/cZipFile path.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "ZipFile.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

farmar::recovered::String ansi(const wchar_t* text) {
    const auto count = WideCharToMultiByte(CP_ACP, 0, text, -1, nullptr, 0, nullptr, nullptr);
    if (!count) throw "Cannot convert command line to original ANSI paths";
    std::string bytes(count, '\0');
    WideCharToMultiByte(CP_ACP, 0, text, -1, bytes.data(), count, nullptr, nullptr);
    return farmar::recovered::String(std::string_view(bytes.data(), count - 1));
}
int wmain(int argc, wchar_t** argv) {
    if (argc != 4) { std::cerr << "Usage: pack-decode archive.pack member output\n"; return 2; }
    try {
        farmar::recovered::cZipFile file(ansi(argv[1]), ansi(argv[2]));
        std::vector<char> bytes(file.size());
        const auto actual = file.read(bytes.data(), file.size());
        if (actual != file.size()) throw "Pack read count differs from entry size";
        std::ofstream output(std::filesystem::path(argv[3]), std::ios::binary);
        output.write(bytes.data(), bytes.size());
        if (!output) throw "Cannot write diagnostic output";
        std::cout << actual << '\n';
    } catch (const char* error) { std::cerr << error << '\n'; return 1; }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
    return 0;
}
