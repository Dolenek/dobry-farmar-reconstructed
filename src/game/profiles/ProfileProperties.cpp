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
WString ProfileProperties::dumpXML() const {
    std::wostringstream stream;
    stream<<L"<?xml version=\"1.0\" encoding=\"UTF-16\"?><Props>";
    const auto write=[&stream](const auto& values,const wchar_t* type,const auto& valueWriter,const wchar_t* ending=L"\" />") {
        for(const auto& property:values) {
            const auto name=toWide(String(property.first));
            stream<<L"<prop Name=\""<<name.c_str()<<L"\" Val=\"";
            valueWriter(stream,property.second);
            stream<<L"\" Type=\""<<type<<ending;
        }
    };
    const auto numeric=[](std::wostream& out,const auto& value) { out<<value; };
    write(floats,L"Float",numeric);write(doubles,L"Double",numeric);write(ints,L"Int",numeric);
    write(uints,L"UInt",numeric);write(longs,L"Long",numeric);write(ulongs,L"ULong",numeric);
    write(longLongs,L"LongLong",numeric);
    // Original writer emits decimal ARGB although the loader reads hexadecimal.
    write(colors,L"DXColor",[](std::wostream& out,const Color& value) { out<<value.toARGB(); });
    write(booleans,L"Bool",numeric);write(uint16s,L"UInt16",numeric);
    // 004e30f0 writes a single space and ends with " >, not the other types' />.
    write(vectors,L"Vector2",[](std::wostream& out,const std::array<float,2>& value) { out<<value[0]<<L' '<<value[1]; },L"\" >");
    write(strings,L"String",[](std::wostream& out,const String& value) { const auto text=toWide(value);out.write(text.data(),text.size()); });
    write(wideStrings,L"WString",[](std::wostream& out,const WString& value) { out.write(value.data(),value.size()); });
    stream<<L"</Props>";return WString(stream.str());
}
}
