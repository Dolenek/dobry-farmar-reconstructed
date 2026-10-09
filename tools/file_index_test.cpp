// Integration test of create / resource invalidation / reopen / remove fallback.
// Paths supplied by validate_resources.py always refer to its isolated fixtures.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "FileIndex.h"
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace farmar::recovered;
String ansi(const wchar_t* text) {
    const auto count=WideCharToMultiByte(CP_ACP, 0, text, -1, nullptr, 0, nullptr, nullptr);
    if (!count) throw std::runtime_error("Path conversion failed");
    std::string result(count, '\0');
    WideCharToMultiByte(CP_ACP, 0, text, -1, result.data(), count, nullptr, nullptr);
    return String(std::string_view(result.data(), count-1));
}
void check(bool value, const char* error) { if (!value) throw std::runtime_error(error); }
int wmain(int argc, wchar_t** argv) {
    if (argc!=3) return 2;
    try {
        FileIndex files(ansi(argv[1]), String("pack"));
        files.mountCommon(ansi(argv[2]));
        unsigned invalidated=0;
        files.onResourceInvalidated([&](const String& name) {
            check(name.view()=="collision.bin" || name.view()=="profiles/alpha/save.bin", "Invalidation name");
            ++invalidated;
        });
        const auto inherited=files.open(String("collision.bin"));
        unsigned char oldByte=0;
        check(inherited->read(&oldByte, 1)==1 && oldByte=='B', "Package baseline");
        inherited->close();
        auto write=files.create(String("COLLISION.BIN"));
        check(write->write("new", 3)==3, "Common file write");
        write.reset();
        check(files.isCommonExist(String("collision.bin")), "New common index entry");
        check(!files.isCommonExist(String("COLLISION.BIN")), "Original case-sensitive common query");
        char result[4]{};
        auto read=files.open(String("COLLISION.BIN"));
        check(read->read(result, 4)==3 && std::string(result)=="new", "Reopen indexed common file");
        read.reset();
        check(files.removeCommon(String("collision.bin")), "Remove common override");
        check(!files.isCommonExist(String("collision.bin")), "Restore original mapping");
        read=files.open(String("collision.bin"));
        result[0]=0;
        check(read->read(result, 1)==1 && result[0]=='B', "Package restored after delete");
        read.reset();
        write=files.create(String("Profiles\\Alpha\\Save.BIN"));
        check(write->write("profile", 7)==7, "Nested profile file write");
        write.reset();
        std::vector<String> names;
        files.getFilesNames(String("profiles/alpha/"), names);
        check(names.size()==1 && names.front().view()=="save.bin", "Enumerate common profile basenames");
        check(files.removeCommon(String("profiles/alpha/save.bin")), "Remove newly created profile fixture");
        check(!files.isExist(String("profiles/alpha/save.bin")), "No nonexistent fallback entry");
        check(!files.removeCommon(String("profiles/alpha/save.bin")), "Repeated removal fails");
        check(invalidated==2, "Creation invalidates resource cache exactly once");
        std::cout << "12\n";
        return 0;
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
