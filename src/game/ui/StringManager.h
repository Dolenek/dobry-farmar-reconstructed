#pragma once
#include "GameWString.h"
#include <map>
#include <set>
#include <string>
namespace farmar::recovered {
class FileIndex;
class Settings;
class ResourceManager;
// Original cStringManager behaviors with modern containers, not its 32-byte ABI.
class StringManager {
public:
    StringManager(FileIndex& files,Settings& settings,ResourceManager& resources); // 0052f230
    ~StringManager() noexcept(false); // 0052e670, optional strdubp.xml
    StringManager(const StringManager&)=delete;
    StringManager& operator=(const StringManager&)=delete;
    const WString& getString(const WString& source) const; // 0052ed20
    const std::map<std::wstring,WString>& translations() const noexcept { return translations_; }
    const std::set<std::wstring>& missingStrings() const noexcept { return missing_; }
private:
    FileIndex& files_;
    bool dump_=false;
    mutable std::map<std::wstring,WString> translations_;
    mutable std::set<std::wstring> missing_;
};
}
