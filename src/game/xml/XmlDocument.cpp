#include "XmlDocument.h"
#include "FileIndex.h"
#include <expat.h>
#include <cstdio>
#include <cwchar>
#include <vector>

namespace farmar::recovered {
namespace {
// 006612d0..006612dc: shared original callback parent deque (not thread-local).
std::vector<XmlNode*> parents;
std::string narrowName(const wchar_t* text) {
    char output[4152]{}; // original 00563910 uses CRT sprintf("%S")
    std::sprintf(output, "%S", text);
    return output;
}
}
bool XmlNode::section(const String& name, XmlNode*& output) const {
    const auto at = children_.find(std::string(name.view()));
    if (at == children_.end()) return false;
    output = at->second.get();
    return true;
}
bool XmlNode::section(std::uint32_t index, XmlNode*& output, String* name) const {
    // Enumeration follows the sorted multimap, retaining duplicate insertion order.
    if (index >= children_.size()) return false;
    auto at = children_.begin();
    std::advance(at, index);
    output = at->second.get();
    if (name) name->assign(at->first.data(), static_cast<String::size_type>(at->first.size()));
    return true;
}
WString XmlNode::attribute(const WString& name) const {
    const auto at = attributes_.find(std::wstring(name.view()));
    return at == attributes_.end() ? WString() : at->second;
}
XmlParseError::XmlParseError(unsigned code, unsigned line, unsigned column, const std::string& message)
    : std::runtime_error(message+" line "+std::to_string(line)+" char "+std::to_string(column)),
      code(code), line(line), column(column) {}
void XmlDocument::start(void* current, const wchar_t* name, const wchar_t** attributes) { // 00563910
    auto* parent = static_cast<XmlNode*>(current);
    const bool nested = !parents.empty();
    parents.push_back(parent);
    XmlNode* node = parent;
    if (nested) {
        auto child = std::make_unique<XmlNode>();
        child->parser_ = parent->parser_;
        node = child.get();
        parent->children_.emplace(narrowName(name), std::move(child));
        XML_SetUserData(static_cast<XML_Parser>(parent->parser_), node);
    }
    // The first element is represented by the document object; its tag is omitted.
    for (auto at=attributes; *at; at+=2) {
        auto& value = node->attributes_[*at];
        value.assign(at[1], static_cast<WString::size_type>(std::wcslen(at[1])));
    }
}
void XmlDocument::end(void*, const wchar_t*) { // 0055f140
    auto* parent = parents.back();
    parents.pop_back();
    XML_SetUserData(static_cast<XML_Parser>(parent->parser_), parent);
}
void XmlDocument::characters(void* current, const wchar_t* text, int count) { // 0055f1e0
    // Original terminates each chunk and appends wcslen units, preserving whitespace.
    const std::wstring terminated(text, count);
    static_cast<XmlNode*>(current)->text_.append(terminated.c_str(),
        static_cast<WString::size_type>(std::wcslen(terminated.c_str())));
}
XmlDocument::XmlDocument(FileIndex& files, const String& name) {
    bool fromCommon = files.isCommonExist(name); // query before open, without folding
    for (;;) {
        auto file = files.open(name);
        std::vector<char> bytes(file->size());
        file->read(bytes.data(), static_cast<std::uint32_t>(bytes.size()));
        file.reset();
        auto parser = XML_ParserCreate(nullptr);
        if (!parser) throw std::bad_alloc();
        parser_ = parser;
        XML_SetElementHandler(parser, start, end);
        XML_SetCharacterDataHandler(parser, characters);
        XML_SetUserData(parser, this);
        const auto result = XML_Parse(parser, bytes.data(), static_cast<int>(bytes.size()), 1);
        if (result) { XML_ParserFree(parser); parser_ = nullptr; return; }
        if (fromCommon) {
            fromCommon = false;
            XML_ParserFree(parser);
            parser_ = nullptr;
            files.removeCommon(name);
            // Original does not clear partial children, text or its shared parent
            // deque before retrying. Partial-element error recovery is unverified.
            continue;
        }
        const auto code = XML_GetErrorCode(parser);
        const auto line = XML_GetCurrentLineNumber(parser);
        const auto column = XML_GetCurrentColumnNumber(parser);
        const auto message = narrowName(XML_ErrorString(code));
        XML_ParserFree(parser);
        parser_ = nullptr;
        throw XmlParseError(code, line, column, message);
    }
}
}
