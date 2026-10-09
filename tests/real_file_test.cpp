#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "RealFile.h"
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>

int main() {
    using farmar::recovered::cRealFile;
    char folder[MAX_PATH], name[MAX_PATH];
    if (!GetTempPathA(MAX_PATH, folder) || !GetTempFileNameA(folder, "dfr", 0, name)) return 2;
    int failures = 0;
    auto check = [&](bool result, const char* label) {
        if (!result) { std::cerr << label << '\n'; ++failures; }
    };
    try {
        cRealFile file(name, GENERIC_READ | GENERIC_WRITE, CREATE_ALWAYS);
        check(file.size() == 0, "new file empty");
        check(file.write("abc\0def", 7) == 7 && file.size() == 7, "binary write");
        check(file.seek(-3, FILE_END) == 4, "signed seek from end");
        char output[16]{};
        check(file.read(output, sizeof(output)) == 3 && std::memcmp(output, "def", 3) == 0, "short read");
        check(file.read(output, 1) == 0, "EOF");
        check(file.seek(2, FILE_BEGIN) == 2 && file.seek(1, FILE_CURRENT) == 3, "seek origins");
        check(file.seek(-1, FILE_BEGIN) == INVALID_SET_FILE_POINTER, "failed seek sentinel");
        file.close();
        check(file.size() == 0 && file.seek(99, FILE_BEGIN) == 0 &&
              file.read(output, 2) == 0 && file.write("x", 1) == 0, "closed file operations");
        file.close();
        {
            cRealFile read_only(name, GENERIC_READ, OPEN_EXISTING);
            check(read_only.write("x", 1) == 0, "read-only write reports zero");
            HANDLE writer = CreateFileA(name, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                        nullptr, OPEN_EXISTING, 0, nullptr);
            check(writer == INVALID_HANDLE_VALUE && GetLastError() == ERROR_SHARING_VIOLATION,
                  "read access grants only read sharing");
            if (writer != INVALID_HANDLE_VALUE) CloseHandle(writer);
        }
        {
            cRealFile write_only(name, GENERIC_WRITE, OPEN_EXISTING);
            check(write_only.read(output, 1) == 0, "write-only read reports zero");
        }
        check(DeleteFileA(name) != 0, "destructor closes handle");
        bool threw = false;
        try { cRealFile missing(name, GENERIC_READ, OPEN_EXISTING); }
        catch (const std::runtime_error& error) {
            threw = true;
            check(error.what() == std::string("Can't open file \"") + name + "\".", "primary error text");
        }
        check(threw, "missing file error");
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; ++failures; }
    DeleteFileA(name);
    return failures ? 1 : 0;
}
