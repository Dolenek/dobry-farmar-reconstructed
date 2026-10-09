#pragma once
#include "GameWString.h"
#include "GameString.h"
#include <map>
#include <vector>
namespace farmar::recovered {
class XmlNode;
// Header portion of cProfile. Typed property stores, switching/create/delete,
// persistence lifecycle and its singleton/88-byte object ABI are separate work.
class ProfileHeaders {
public:
    struct Less { bool operator()(const WString& a,const WString& b) const { return a.compare(0,WString::npos,b.data(),b.size())<0; } };
    using Names=std::map<WString,unsigned,Less>;
    explicit ProfileHeaders(const WString& requestedName=WString()); // data defaults of 004ecb20
    void loadXML(const XmlNode& node); // 004ea000, merges names
    void appendNames(std::vector<WString>& output) const; // 004e8900, appends
    bool findID(const WString& name,unsigned& output) const; // 004dfd90 lookup
    String currentID() const; // 004e4110, 004e17e0 formatting
    const Names& names() const noexcept { return names_; }
    const WString& storedCurrentName() const noexcept { return storedCurrentName_; } // native +8
    const WString& currentName() const noexcept { return currentName_; }
    unsigned nextID() const noexcept { return nextID_; }
    bool nowPlaying() const noexcept { return nowPlaying_; }
    bool dirty() const noexcept { return dirty_; }
private:
    Names names_;WString storedCurrentName_,currentName_; // +8 / +24 in original
    unsigned nextID_=1;bool nowPlaying_=false,dirty_=false;
};
String formatProfileID(unsigned id); // decimal, zero fill, minimum width eight (004e17e0)
}
