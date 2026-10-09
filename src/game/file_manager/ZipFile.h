#pragma once
#include "RealFile.h"

namespace farmar::recovered {
class cZipFile final : public iFile {
public:
    cZipFile(const String& archive, const String& name);
    ~cZipFile() override;
    cZipFile(const cZipFile&) = delete;
    cZipFile& operator=(const cZipFile&) = delete;
    std::uint32_t size() const override;
    std::uint32_t seek(std::int32_t offset, std::uint32_t origin) override;
    std::uint32_t read(void* output, std::uint32_t count) override;
    std::uint32_t write(const void* input, std::uint32_t count) override;
    void close() override;
    using ZIP_FILE = void*;
    static ZIP_FILE findOpen(const String& path);
    static void findClose(ZIP_FILE finder);
    static bool findFirstFile(ZIP_FILE finder, String& name);
    static bool findNextFile(ZIP_FILE finder, String& name);
private:
    void* archive_;
    std::uint32_t size_;
};
static_assert(sizeof(cZipFile) == 12, "Original x86 ZIP file layout");
}
