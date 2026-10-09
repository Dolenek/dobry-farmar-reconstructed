#include "ProfileHeaders.h"
#include "XmlDocument.h"
#include <iomanip>
#include <sstream>
namespace farmar::recovered {
namespace {
template<class T> void read(const WString& text,T& value) {
    if(!text.empty()) { std::wistringstream stream(text.c_str());stream>>value; }
}
}
ProfileHeaders::ProfileHeaders(const WString& requestedName):currentName_(requestedName) {}
void ProfileHeaders::loadXML(const XmlNode& node) {
    read(node.attribute(WString(L"NextID")),nextID_);
    storedCurrentName_=node.attribute(WString(L"CurrentUser"));
    read(node.attribute(WString(L"NowPlaying")),nowPlaying_);
    for(unsigned i=0;i<node.children().size();++i) {
        XmlNode* child=nullptr;
        if(node.section(i,child)) {
            const auto name=child->attribute(WString(L"Name"));unsigned id=0;
            read(child->attribute(WString(L"ID")),id);
            // Original unique insert preserves the FIRST ID for duplicate names,
            // including entries left by earlier loadHeaders calls.
            names_.emplace(name,id);
        }
    }
    dirty_=false;
}
void ProfileHeaders::appendNames(std::vector<WString>& output) const { for(const auto& entry:names_) output.push_back(entry.first); }
bool ProfileHeaders::findID(const WString& name,unsigned& output) const {
    const auto found=names_.find(name);if(found==names_.end()) return false;output=found->second;return true;
}
String formatProfileID(unsigned id) {
    std::ostringstream stream;stream<<std::setfill('0')<<std::setw(8)<<id;
    const auto result=stream.str();return String(std::string_view(result));
}
String ProfileHeaders::currentID() const {
    unsigned id=0;if(!findID(currentName_,id)) throw "ProfileID error.";
    return formatProfileID(id);
}
}
