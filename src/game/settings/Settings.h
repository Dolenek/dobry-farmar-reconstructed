#pragma once
#include "GameString.h"
#include "GameWString.h"
#include "GameColor.h"
#include <map>
#include <string>
#include <functional>
#include <memory>

namespace farmar::recovered {
class FileIndex;
class XmlNode;
// Recovered cSettings data operations. Original singleton, vtable, exceptions and
// non-window property types are not yet the complete engine ABI.
class Settings {
public:
    using Properties = std::map<std::string, WString>;
    using Sections = std::map<std::string, Properties>;
    explicit Settings(FileIndex& files); // 0053ee00, default sys/settings.xml
    using XmlLoader=std::function<std::shared_ptr<const XmlNode>(const String&)>;
    Settings(FileIndex& files,XmlLoader loader); // recovered resource-manager call site
    void load(const String& filename); // 00538610, merges rather than clears
    void save(); // 00537240, compact unescaped UTF-16LE XML
    bool get(const String& section, const String& property, WString& value); // 00539420
    bool get(const String& section, const String& property, String& value); // 00538bb0
    bool get(const String& section, const String& property, int& value); // 0053bcd0
    bool get(const String& section, const String& property, unsigned& value); // 0053c690
    bool get(const String& section, const String& property, float& value); // 0053e3d0
    bool get(const String& section, const String& property, Color& value); // 00539c30 / 004d93b0
    bool get(const String& section, const String& property, bool& value); // 0053b300
    void set(const String& section, const String& property, const WString& value); // 00539850
    void set(const String& section, const String& property, const String& value); // 00539010
    void set(const String& section, const String& property, int value); // 0053c1d0
    void set(const String& section, const String& property, unsigned value); // 0053cb90
    void set(const String& section, const String& property, float value); // 0053e8d0
    void set(const String& section, const String& property, bool value); // 0053b800
    const Sections& sections() const noexcept { return sections_; }
private:
    FileIndex& files_;
    String filename_{"sys/settings.xml"};
    Sections sections_;
    XmlLoader xmlLoader_;
    // Reading an absent property inserts it and returns false; output is untouched.
    bool lookup(const String& section, const String& property, const WString*& value);
};
}
