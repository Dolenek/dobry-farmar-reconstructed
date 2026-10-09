#include "ZipFile.h"
#include <unzip.h>
#include <stdexcept>
#include <string>

namespace farmar::recovered {
namespace {
char first_name[256]{}; // 006612e0
char next_name[256]{};  // 006613e0
}
cZipFile::cZipFile(const String& archive, const String& name) : archive_(nullptr), size_(0) {
    // 00565510; the misspelling in the original errors is intentional.
    archive_ = unzOpen(archive.c_str());
    if (!archive_) throw std::runtime_error("Cant't open pack file '" + std::string(archive.view()) + "'");
    if (unzLocateFile(archive_, name.c_str(), 0) != UNZ_OK)
        throw std::runtime_error("Cant't find file '" + std::string(name.view()) + "' in the '" + std::string(archive.view()) + "' pack.");
    if (unzOpenCurrentFile(archive_) != UNZ_OK)
        throw std::runtime_error("Cant't open file '" + std::string(name.view()) + "' in the '" + std::string(archive.view()) + "' pack.");
    unz_file_info info{};
    unzGetCurrentFileInfo(archive_, &info, nullptr, 0, nullptr, 0, nullptr, 0);
    size_ = info.uncompressed_size;
}
cZipFile::~cZipFile() { close(); } // 00566f90
std::uint32_t cZipFile::size() const { return size_; } // 00565ce0
std::uint32_t cZipFile::seek(std::int32_t offset, std::uint32_t origin) { // 00565d00
    if (!archive_) return 0;
    // Original uses DIRECTORY offsets, not uncompressed stream positions.
    if (origin == 0) unzSetOffset(archive_, static_cast<std::uint32_t>(offset));
    else if (origin == 1) unzSetOffset(archive_, unzGetOffset(archive_) + static_cast<std::uint32_t>(offset));
    else if (origin == 2) unzSetOffset(archive_, size_ + static_cast<std::uint32_t>(offset));
    return unzGetOffset(archive_);
}
std::uint32_t cZipFile::read(void* output, std::uint32_t count) { // 00565da0
    return archive_ ? static_cast<std::uint32_t>(unzReadCurrentFile(archive_, output, count)) : 0;
}
std::uint32_t cZipFile::write(const void*, std::uint32_t) { return 0; } // 00565cf0
void cZipFile::close() { // 00565e00
    if (archive_) { unzCloseCurrentFile(archive_); unzClose(archive_); archive_ = nullptr; }
}
cZipFile::ZIP_FILE cZipFile::findOpen(const String& path) { // 00565e60
    auto finder = unzOpen(path.c_str());
    if (!finder) throw std::runtime_error(std::string(path.view()) + " is cannot be opened.");
    return finder;
}
void cZipFile::findClose(ZIP_FILE finder) { // 005662b0
    if (unzClose(finder) != UNZ_OK) throw std::runtime_error("Error has occurred while closing zip find.");
}
bool cZipFile::findFirstFile(ZIP_FILE finder, String& name) { // 005666b0
    const auto result = unzGoToFirstFile(finder);
    if (result == UNZ_END_OF_LIST_OF_FILE) return false;
    if (result != UNZ_OK) throw "Pack finder raise error during begin of search.";
    if (unzGetCurrentFileInfo(finder, nullptr, first_name, 256, nullptr, 0, nullptr, 0) != UNZ_OK)
        throw "Pack finder raise error during begin of search. (File name can't be obtained.)";
    name.assign(String(first_name));
    return true;
}
bool cZipFile::findNextFile(ZIP_FILE finder, String& name) { // 00566b20
    const auto result = unzGoToNextFile(finder);
    if (result == UNZ_END_OF_LIST_OF_FILE) return false;
    if (result != UNZ_OK) throw "Pack finder raise error during search.";
    if (unzGetCurrentFileInfo(finder, nullptr, next_name, 256, nullptr, 0, nullptr, 0) != UNZ_OK)
        throw "Pack finder raise error during search. (File name can't be obtained.)";
    name.assign(String(next_name));
    return true;
}
}
