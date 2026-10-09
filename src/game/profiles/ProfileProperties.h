#pragma once
#include "GameString.h"
#include "GameWString.h"
#include "GameColor.h"
#include <array>
#include <cstdint>
#include <map>
#include <string>
namespace farmar::recovered {
class XmlNode;
// Thirteen distinct typed cProfile stores. Original templates insert a default
// value when a key is requested. They are process-global, rather than members
// of the 88-byte cProfile object.
class ProfileProperties {
public:
    template<class T> using Values=std::map<std::string,T>;
    Values<float> floats; // 0065ff38, get-or-insert 004106a0
    Values<double> doubles; // 00660464, 004e9a70
    Values<std::int32_t> ints; // 0065ff20, 00410770
    Values<std::uint32_t> uints; // 0065ff44, 00410770
    Values<std::int32_t> longs; // 00660470, 004e9b40
    Values<std::uint32_t> ulongs; // 0066047c, 004e9b40
    Values<std::int64_t> longLongs; // 00660488, 004e9c00
    Values<Color> colors; // 00660458, 004e9990
    Values<bool> booleans; // 0065ff2c, 004105e0
    Values<std::uint16_t> uint16s; // 00660494, 004e9cc0
    Values<std::array<float,2>> vectors; // 00660434, 004e98c0
    Values<String> strings; // 00660440, 004eac20
    Values<WString> wideStrings; // 0066044c, 004eb1d0
    void clear(); // 004e9d80
    void setProperty(const String& type,const String& name,const WString& value); // 004ebd10
    void loadXML(const XmlNode& node); // property loop of 004ec310
    WString dumpXML() const; // 004e6cf0 and its thirteen typed serializers
};
ProfileProperties& profileProperties();
}
