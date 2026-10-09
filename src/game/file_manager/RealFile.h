#pragma once
#include "GameString.h"
#include <cstdint>

namespace farmar::recovered {
// Slot order recovered from 00601e40..00601e54 (Windows x86).
class iFile {
public:
    virtual ~iFile() = default;
    virtual std::uint32_t size() const = 0;
    virtual std::uint32_t seek(std::int32_t offset, std::uint32_t origin) = 0;
    virtual std::uint32_t read(void* output, std::uint32_t count) = 0;
    virtual std::uint32_t write(const void* input, std::uint32_t count) = 0;
    virtual void close() = 0;
};

// File I/O implementation recovered; engine exception wrapping remains pending.
class cRealFile final : public iFile {
public:
    cRealFile(const String& name, std::uint32_t access, std::uint32_t creating);
    explicit cRealFile(void* existing_handle) noexcept; // 00564800: adopts HANDLE
    ~cRealFile() override;
    cRealFile(const cRealFile&) = delete;
    cRealFile& operator=(const cRealFile&) = delete;
    std::uint32_t size() const override;
    std::uint32_t seek(std::int32_t offset, std::uint32_t origin) override;
    std::uint32_t read(void* output, std::uint32_t count) override;
    std::uint32_t write(const void* input, std::uint32_t count) override;
    void close() override;
private:
    void* handle_;
};
static_assert(sizeof(void*) == 4 && sizeof(cRealFile) == 8, "Original x86 file layout");
}
