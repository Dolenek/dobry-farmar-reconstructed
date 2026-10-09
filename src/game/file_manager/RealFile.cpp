#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "RealFile.h"
#include <stdexcept>
#include <string>

namespace farmar::recovered {
cRealFile::cRealFile(void* existing_handle) noexcept : handle_(existing_handle) {}
// 00564820: the share flags follow requested access (not a generic share-all).
cRealFile::cRealFile(const String& name, std::uint32_t access, std::uint32_t creating)
    : handle_(nullptr) {
    DWORD sharing = (access & GENERIC_READ) ? FILE_SHARE_READ : 0;
    if (access & GENERIC_WRITE) sharing |= FILE_SHARE_WRITE;
    handle_ = CreateFileA(name.c_str(), access, sharing, nullptr, creating, 0, nullptr);
    if (handle_ == INVALID_HANDLE_VALUE)
        throw std::runtime_error("Can't open file \"" + std::string(name.view()) + "\".");
}
cRealFile::~cRealFile() { close(); } // 00564e60
std::uint32_t cRealFile::size() const { // 00564c90
    return handle_ ? GetFileSize(handle_, nullptr) : 0;
}
std::uint32_t cRealFile::seek(std::int32_t offset, std::uint32_t origin) { // 00564ce0
    return handle_ ? SetFilePointer(handle_, offset, nullptr, origin) : 0;
}
std::uint32_t cRealFile::read(void* output, std::uint32_t count) { // 00564d40
    if (!handle_) return 0;
    DWORD transferred = count;
    ReadFile(handle_, output, count, &transferred, nullptr);
    return transferred;
}
std::uint32_t cRealFile::write(const void* input, std::uint32_t count) { // 00564da0
    if (!handle_) return 0;
    DWORD transferred = count;
    WriteFile(handle_, input, count, &transferred, nullptr);
    return transferred;
}
void cRealFile::close() { // 00564e00: clear the handle even if CloseHandle fails.
    if (handle_) { CloseHandle(handle_); handle_ = nullptr; }
}
}
