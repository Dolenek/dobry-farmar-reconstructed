#include "Settings.h"
#include "FileIndex.h"
#include "StringConversion.h"
#include "XmlDocument.h"
#include <sstream>

namespace farmar::recovered {
namespace {
std::string key(const String& value) { return std::string(value.view()); }
template<class Value> WString format(Value value) {
    std::wostringstream stream;
    stream << value; // original wstringstream, default numeric flags (no boolalpha)
    return WString(stream.str());
}
template<class Value> void extract(const WString& text, Value& value) {
    // Original copies from c_str(), not the stored length, then performs operator>>.
    std::wistringstream stream(text.c_str());
    stream >> value;
}
}
Settings::Settings(FileIndex& files) : Settings(files,{}) {}
Settings::Settings(FileIndex& files,XmlLoader loader) : files_(files),xmlLoader_(std::move(loader)) { load(filename_); }
void Settings::load(const String& filename) {
    auto document=xmlLoader_ ? xmlLoader_(filename) : std::make_shared<XmlDocument>(files_,filename);
    for (const auto& section : document->children()) {
        for (const auto& attribute : section.second->attributes()) {
            const auto name = toNarrow(WString(attribute.first));
            sections_[section.first][key(name)] = attribute.second;
        }
    }
}
bool Settings::lookup(const String& section, const String& property, const WString*& value) {
    auto at = sections_.find(key(section));
    if (at != sections_.end()) {
        auto item = at->second.find(key(property));
        if (item != at->second.end()) { value = &item->second; return true; }
    }
    value = &sections_[key(section)][key(property)];
    return false;
}
bool Settings::get(const String& section, const String& property, WString& value) {
    const WString* text;
    if (!lookup(section, property, text)) return false;
    value = *text; return true;
}
bool Settings::get(const String& section, const String& property, String& value) {
    const WString* text;
    if (!lookup(section, property, text)) return false;
    value = toNarrow(*text); return true;
}
bool Settings::get(const String& section, const String& property, int& value) {
    const WString* text;
    if (!lookup(section, property, text)) return false;
    extract(*text, value); return true; // success reports existence, ignores failbit
}
bool Settings::get(const String& section, const String& property, unsigned& value) {
    const WString* text;
    if (!lookup(section,property,text)) return false;
    extract(*text,value); return true;
}
bool Settings::get(const String& section, const String& property, float& value) {
    const WString* text;
    if(!lookup(section,property,text)) return false;
    extract(*text,value);return true;
}
bool Settings::get(const String& section, const String& property, Color& value) {
    const WString* text;
    if(!lookup(section,property,text)) return false;
    if(!text->empty()) {
        unsigned packed=0;
        std::wistringstream stream(text->c_str());
        stream >> std::hex >> packed; // 004d93b0 clears dec 0x200 and sets hex 0x800
        value=Color::fromARGB(packed);
    }
    return true;
}
bool Settings::get(const String& section, const String& property, bool& value) {
    const WString* text;
    if (!lookup(section, property, text)) return false;
    extract(*text, value); return true;
}
void Settings::set(const String& section, const String& property, const WString& value) {
    sections_[key(section)][key(property)] = value;
}
void Settings::set(const String& section, const String& property, const String& value) {
    set(section, property, toWide(value));
}
void Settings::set(const String& section, const String& property, int value) {
    set(section, property, format(value));
}
void Settings::set(const String& section, const String& property, unsigned value) {
    set(section,property,format(value));
}
void Settings::set(const String& section, const String& property, float value) {
    set(section,property,format(value));
}
void Settings::set(const String& section, const String& property, bool value) {
    set(section, property, format(value));
}
void Settings::save() {
    std::wostringstream stream;
    stream << L"<?xml version=\"1.0\" encoding=\"UTF-16\"?><main>";
    for (const auto& section : sections_) {
        stream << L'<' << section.first.c_str() << L' ';
        for (const auto& property : section.second)
            stream << property.first.c_str() << L"=\"" << property.second.c_str() << L"\" ";
        stream << L"/>";
    }
    stream << L"</main>";
    const auto contents = stream.str();
    auto file = files_.create(filename_);
    const wchar_t bom = 0xfeff; // 0050ce30 followed by 0050ca60
    file->write(&bom, 2);
    file->write(contents.data(), static_cast<std::uint32_t>(contents.size() * 2));
}
}
