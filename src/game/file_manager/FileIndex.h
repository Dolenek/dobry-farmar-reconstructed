#pragma once
#include "GameString.h"
#include "RealFile.h"
#include <map>
#include <memory>
#include <functional>
#include <set>
#include <string>
#include <vector>

namespace farmar::recovered {
// Recovered file-index operations, separated from the not-yet-complete manager
// singleton / engine-resource ABI. All paths retain original ANSI conventions.
class FileIndex {
public:
    using Table = std::map<std::string, std::string>;
    FileIndex(const String& root, const String& pack_extension);
    bool isExist(const String& name) const; // 005489b0
    bool isCommonExist(const String& name) const; // 0054a890 (case-sensitive query)
    std::unique_ptr<iFile> open(const String& name) const; // 00548de0
    void mountCommon(const String& root); // indexing / merging portion of 0054d720
    bool removeCommon(const String& name); // 0054d5c0
    std::unique_ptr<iFile> create(const String& name); // 0054c7d0, ANSI overload
    void getFilesNames(const String& path, std::vector<String>& output) const; // 0054b4b0
    static void createDirectories(const String& path); // 00549f70
    // Restored call site for resource-manager release on creation, 0065f730 +1c.
    void onResourceInvalidated(std::function<void(const String&)> callback) { invalidated_=std::move(callback); }
    const Table& files() const noexcept { return files_; }
    const String& root() const noexcept { return root_; }
private:
    String root_, extension_, common_;
    Table files_, original_;
    std::set<std::string> packs_;
    std::function<void(const String&)> invalidated_;
    void indexDirectory(const String& root, const std::string& tag); // 0054bd80
    void indexPacks(); // 0054d720
};
}
