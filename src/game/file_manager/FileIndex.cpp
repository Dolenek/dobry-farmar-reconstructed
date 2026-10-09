#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "FileIndex.h"
#include "PathNames.h"
#include "ZipFile.h"
#include <algorithm>
#include <stdexcept>
#include <vector>

namespace farmar::recovered {
namespace {
String lower(const String& name) { String result(name); result.lowercase(); return result; }
String directory(const String& name, bool lowercase) {
    String result(name);
    if (lowercase) result.lowercase();
    std::replace(result.data(), result.data()+result.size(), '\\', '/');
    if (!result.empty() && result.data()[result.size()-1] != '/') {
        const auto joined = std::string(result.view())+'/';
        result.assign(joined.data(), static_cast<String::size_type>(joined.size()));
    }
    return result;
}
String joined(const String& root, const std::string& suffix) {
    return String(std::string_view(std::string(root.view())+suffix));
}
struct OriginalSecurity {
    SECURITY_DESCRIPTOR descriptor;
    SECURITY_ATTRIBUTES attributes;
    OriginalSecurity() {
        InitializeSecurityDescriptor(&descriptor, SECURITY_DESCRIPTOR_REVISION);
        SetSecurityDescriptorDacl(&descriptor, TRUE, nullptr, FALSE);
        attributes={sizeof(SECURITY_ATTRIBUTES), &descriptor, TRUE};
    }
};
}
FileIndex::FileIndex(const String& root, const String& pack_extension)
    : root_(directory(root, true)), extension_(lower(pack_extension)) {
    indexDirectory(root_, "");
    indexPacks();
}
void FileIndex::indexDirectory(const String& root, const std::string& tag) {
    std::vector<std::string> pending{std::string{}};
    while (!pending.empty()) {
        const auto relative = pending.back();
        pending.pop_back();
        WIN32_FIND_DATAA entry{};
        auto finder = FindFirstFileA(joined(root, relative+'*').c_str(), &entry);
        if (finder != INVALID_HANDLE_VALUE) {
            do {
                if (entry.cFileName[0] == '.') continue;
                if (entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    pending.push_back(relative+entry.cFileName+'/');
                } else {
                    // The original stores only the basename for packs, even in
                    // nested folders. This is not changed into a relative path.
                    if (PathNames::getFileExt(String(entry.cFileName)).view() == extension_.view())
                        packs_.insert(entry.cFileName);
                    else {
                        auto name = lower(String(std::string_view(relative+entry.cFileName)));
                        files_[std::string(name.view())] = tag;
                    }
                }
            } while (FindNextFileA(finder, &entry));
        }
        FindClose(finder);
    }
}
void FileIndex::indexPacks() {
    for (const auto& pack : packs_) {
        const auto finder = cZipFile::findOpen(joined(root_, pack));
        try {
            String name;
            bool found = cZipFile::findFirstFile(finder, name);
            while (found) {
                name.lowercase();
                if (!PathNames::getFileName(name).empty()) {
                    const auto key = std::string(name.view());
                    const auto existing = files_.find(key);
                    if (existing == files_.end() || !existing->second.empty()) files_[key] = pack;
                }
                found = cZipFile::findNextFile(finder, name);
            }
        } catch (...) { cZipFile::findClose(finder); throw; }
        cZipFile::findClose(finder);
    }
}
bool FileIndex::isExist(const String& name) const {
    return files_.find(std::string(lower(name).view())) != files_.end();
}
bool FileIndex::isCommonExist(const String& name) const {
    const auto at = files_.find(std::string(name.view()));
    return at != files_.end() && at->second == "#common#";
}
std::unique_ptr<iFile> FileIndex::open(const String& name) const {
    const auto key = lower(name);
    const auto at = files_.find(std::string(key.view()));
    if (at == files_.end()) throw std::runtime_error("File \""+std::string(key.view())+"\" not found.");
    if (at->second.empty()) return std::make_unique<cRealFile>(joined(root_, at->first), GENERIC_READ, OPEN_EXISTING);
    if (at->second == "#common#") return std::make_unique<cRealFile>(joined(common_, at->first), GENERIC_READ, OPEN_EXISTING);
    return std::make_unique<cZipFile>(joined(root_, at->second), key);
}
void FileIndex::mountCommon(const String& root) {
    common_ = directory(root, false);
    original_ = files_;
    files_.clear();
    indexDirectory(common_, "#common#");
    files_.insert(original_.begin(), original_.end()); // 0054bbb0: unique insertion
}
bool FileIndex::removeCommon(const String& name) {
    if (!DeleteFileA(joined(common_, std::string(name.view())).c_str())) return false;
    const auto key = std::string(lower(name).view());
    const auto at = files_.find(key);
    if (at != files_.end() && at->second == "#common#") {
        const auto original = original_.find(key);
        if (original == original_.end()) files_.erase(at);
        else at->second = original->second;
    }
    return true;
}
void FileIndex::createDirectories(const String& path) {
    auto start=0U;
    const char slash='/';
    for (;;) {
        const auto at=path.find(&slash, start, 1);
        if (at==String::npos) break;
        OriginalSecurity security;
        CreateDirectoryA(path.substr(0, at).c_str(), &security.attributes);
        start=at+1;
    }
}
std::unique_ptr<iFile> FileIndex::create(const String& name) {
    auto key=lower(name);
    std::replace(key.data(), key.data()+key.size(), '\\', '/');
    const auto path=joined(common_, std::string(key.view()));
    createDirectories(path);
    OriginalSecurity security;
    auto handle=CreateFileA(path.c_str(), GENERIC_READ|GENERIC_WRITE, 0, &security.attributes,
                           CREATE_ALWAYS, 0, nullptr);
    if (handle==INVALID_HANDLE_VALUE)
        throw std::runtime_error("File \""+std::string(name.view())+"\" can't be creating.");
    files_[std::string(key.view())]="#common#";
    if (invalidated_) invalidated_(key);
    return std::make_unique<cRealFile>(handle);
}
void FileIndex::getFilesNames(const String& path, std::vector<String>& output) const {
    WIN32_FIND_DATAA entry{};
    auto finder=FindFirstFileA(joined(common_, std::string(path.view())+'*').c_str(), &entry);
    if (finder!=INVALID_HANDLE_VALUE) {
        do {
            if (entry.cFileName[0]!='.' && !(entry.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) &&
                PathNames::getFileExt(String(entry.cFileName)).view()!=extension_.view())
                output.emplace_back(entry.cFileName);
        } while (FindNextFileA(finder, &entry));
    }
    FindClose(finder);
}
}
