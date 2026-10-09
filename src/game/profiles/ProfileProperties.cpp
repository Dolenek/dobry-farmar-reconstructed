#include "ProfileProperties.h"
#include "StringConversion.h"
#include "XmlDocument.h"
#include <sstream>
namespace farmar::recovered {
namespace {
template<class T> T read(const WString& text) {
    T result{};std::wistringstream stream(text.c_str());stream>>result;return result;
}
}
ProfileProperties& profileProperties() { static ProfileProperties values;return values; }
void ProfileProperties::clear() {
    floats.clear();doubles.clear();ints.clear();uints.clear();longs.clear();ulongs.clear();longLongs.clear();
    colors.clear();booleans.clear();uint16s.clear();vectors.clear();strings.clear();wideStrings.clear();
}
void ProfileProperties::setProperty(const String& type,const String& name,const WString& value) {
    const auto tag=type.view();const std::string key(name.view());
    if(tag=="Float") floats[key]=read<float>(value);
    else if(tag=="Double") doubles[key]=read<double>(value);
    else if(tag=="Int") ints[key]=read<std::int32_t>(value);
    else if(tag=="UInt") uints[key]=read<std::uint32_t>(value);
    else if(tag=="Long") longs[key]=read<std::int32_t>(value);
    else if(tag=="ULong") ulongs[key]=read<std::uint32_t>(value);
    else if(tag=="LongLong") longLongs[key]=read<std::int64_t>(value);
    else if(tag=="DXColor") {
        std::uint32_t packed=0;std::wistringstream stream(value.c_str());stream>>std::hex>>packed;colors[key]=Color::fromARGB(packed);
    }
    else if(tag=="Bool") booleans[key]=read<bool>(value);
    else if(tag=="UInt16") uint16s[key]=read<std::uint16_t>(value);
    else if(tag=="Vector2") {
        std::array<float,2> vector{};std::wistringstream stream(value.c_str());stream>>vector[0];stream.ignore(1);stream>>vector[1];vectors[key]=vector;
    }
    else if(tag=="String") strings[key]=toNarrow(value);
    else if(tag=="WString") wideStrings[key]=value;
    // Original ignores unknown type names without inserting any property.
}
void ProfileProperties::loadXML(const XmlNode& node) {
    for(unsigned i=0;i<node.children().size();++i) {
        XmlNode* child=nullptr;
        if(node.section(i,child)) setProperty(toNarrow(child->attribute(WString(L"Type"))),
            toNarrow(child->attribute(WString(L"Name"))),child->attribute(WString(L"Val")));
    }
}
}
