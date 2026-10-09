#pragma once
#include "GameString.h"
#include "GameWString.h"
#include <map>
#include <memory>
#include <stdexcept>
#include <string>

namespace farmar::recovered {
class FileIndex;
class XmlDocument;
class XmlNode {
public:
    using Children = std::multimap<std::string, std::unique_ptr<XmlNode>>;
    using Attributes = std::map<std::wstring, WString>;
    bool section(const String& name, XmlNode*& output) const; // 0055df00
    bool section(std::uint32_t index, XmlNode*& output, String* name = nullptr) const; // 0055df40
    WString attribute(const WString& name) const; // 0055e860
    const WString& text() const noexcept { return text_; } // 00561320
    const Children& children() const noexcept { return children_; }
    const Attributes& attributes() const noexcept { return attributes_; }
private:
    friend class XmlDocument;
    Children children_;
    Attributes attributes_;
    WString text_;
    void* parser_ = nullptr; // original child constructor 005637b0 receives parser
};
class XmlParseError : public std::runtime_error {
public:
    XmlParseError(unsigned code, unsigned line, unsigned column, const std::string& message);
    unsigned code, line, column;
};
// cXML parsing/tree subset. Engine resource inheritance, serialization and typed
// accessors remain separate recovery work; this is not the complete original ABI.
class XmlDocument final : public XmlNode {
public:
    XmlDocument(FileIndex& files, const String& name); // parsing portion of 00564010
private:
    static void start(void* current, const wchar_t* name, const wchar_t** attributes);
    static void end(void* current, const wchar_t* name);
    static void characters(void* current, const wchar_t* text, int count);
};
}
