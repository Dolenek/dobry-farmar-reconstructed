// Decoder diagnostic for locally supplied images. Does not launch the game.
#include "JNGLoad.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

#if defined(_WIN32)
int wmain(int argc, wchar_t** argv) {
#else
int main(int argc, char** argv) {
#endif
    if (argc != 3) {
        std::cerr << "Usage: jng-decode input.jng output.bgra\n";
        return 2;
    }
    std::ifstream input(std::filesystem::path(argv[1]), std::ios::binary);
    if (!input) { std::cerr << "Cannot open input\n"; return 2; }
    std::vector<unsigned char> bytes{std::istreambuf_iterator<char>(input), {}};
    if (bytes.empty()) { std::cerr << "Input is empty\n"; return 2; }
    // Only complete reference assets are used; the legacy API lacks a length parameter.
    bytes.resize(bytes.size() + 64, 0);
    JngULong width = 0, height = 0;
    JngULong* pixels = nullptr;
    try {
        readMNG(bytes.data(), width, height, pixels);
        std::ofstream output(std::filesystem::path(argv[2]), std::ios::binary);
        if (!output) { freeData(pixels); return 2; }
        output.write(reinterpret_cast<const char*>(pixels), static_cast<std::streamsize>(width) * height * 4);
        freeData(pixels);
        if (!output) return 2;
        std::cout << width << " " << height << "\n";
    } catch (const char* message) {
        std::cerr << message << "\n";
        return 1;
    } catch (const std::exception& error) {
        std::cerr << error.what() << "\n";
        return 1;
    }
    return 0;
}
